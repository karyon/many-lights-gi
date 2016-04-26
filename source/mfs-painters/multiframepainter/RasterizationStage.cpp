#include "RasterizationStage.h"

#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>
#include <glbinding/gl/boolean.h>

#include <globjects/Framebuffer.h>
#include <globjects/Texture.h>
#include <globjects/Program.h>
#include <globjects/Shader.h>

#include <gloperate/base/make_unique.hpp>
#include <gloperate/painter/AbstractPerspectiveProjectionCapability.h>
#include <gloperate/painter/AbstractViewportCapability.h>
#include <gloperate/painter/AbstractCameraCapability.h>

#include <gloperate/primitives/PolygonalDrawable.h>

#include <reflectionzeug/property/extensions/GlmProperties.h>

#include "NoiseTexture.h"
#include "Shadowmap.h"
#include "GroundPlane.h"
#include "Material.h"
#include "ModelLoadingStage.h"
#include "KernelGenerationStage.h"
#include "MultiFramePainter.h"

using namespace gl;
using gloperate::make_unique;

namespace
{
    enum Sampler
    {
        ShadowSampler,
        MaskSampler,
        NoiseSampler,
        DiffuseSampler,
        SpecularSampler,
        EmissiveSampler,
        OpacitySampler,
        BumpSampler
    };
}

RasterizationStage::RasterizationStage(ModelLoadingStage& modelLoadingStage, KernelGenerationStage& kernelGenerationStage)
: m_modelLoadingStage(modelLoadingStage)
, m_kernelGenerationStage(kernelGenerationStage)
, m_shadowmap(nullptr)
, m_presetInformation(modelLoadingStage.getCurrentPreset())
{
    currentFrame = 1;
}
RasterizationStage::~RasterizationStage()
{
}

void RasterizationStage::initProperties(MultiFramePainter& painter)
{
    painter.addProperty<glm::vec3>("LightPosition",
        [this]() { return m_lightPosition; },
        [this](const glm::vec3 & pos) {
            m_lightPosition = pos;
        });
    painter.addProperty<glm::vec3>("LightDirection",
        [this]() { return m_lightDirection; },
        [this](const glm::vec3 & dir) {
            m_lightDirection = dir;
        });
}

void RasterizationStage::initialize()
{
    setupGLState();

    m_shadowmap = make_unique<Shadowmap>();

    diffuseBuffer = globjects::Texture::createDefault(GL_TEXTURE_2D);
    specularBuffer = globjects::Texture::createDefault(GL_TEXTURE_2D);
    faceNormalBuffer = globjects::Texture::createDefault(GL_TEXTURE_2D);
    normalBuffer = globjects::Texture::createDefault(GL_TEXTURE_2D);
    worldPosBuffer = globjects::Texture::createDefault(GL_TEXTURE_2D);
    depthBuffer = globjects::Texture::createDefault(GL_TEXTURE_2D);

    m_fbo = new globjects::Framebuffer();
    m_fbo->attachTexture(GL_COLOR_ATTACHMENT0, diffuseBuffer);
    m_fbo->attachTexture(GL_COLOR_ATTACHMENT1, specularBuffer);
    m_fbo->attachTexture(GL_COLOR_ATTACHMENT2, faceNormalBuffer);
    m_fbo->attachTexture(GL_COLOR_ATTACHMENT3, normalBuffer);
    m_fbo->attachTexture(GL_COLOR_ATTACHMENT4, worldPosBuffer);
    m_fbo->attachTexture(GL_DEPTH_ATTACHMENT, depthBuffer);

    m_program = new globjects::Program();
    m_program->attach(
        globjects::Shader::fromFile(GL_VERTEX_SHADER, "data/shaders/model.vert"),
        globjects::Shader::fromFile(GL_FRAGMENT_SHADER, "data/shaders/model.frag")
    );


    m_zOnlyProgram = new globjects::Program();
    m_zOnlyProgram->attach(
        globjects::Shader::fromFile(GL_VERTEX_SHADER, "data/shaders/model.vert"),
        globjects::Shader::fromFile(GL_FRAGMENT_SHADER, "data/shaders/empty.frag")
    );

    m_modelLoadingStage.process();

    m_groundPlane = make_unique<GroundPlane>(m_presetInformation.groundHeight);

    camera->setEye(m_presetInformation.camEye);
    camera->setCenter(m_presetInformation.camCenter);
    projection->setZNear(m_presetInformation.nearFar.x);
    projection->setZFar(m_presetInformation.nearFar.y);

    m_lightPosition = { 450.0, 1870.0, -250.0 };
    m_lightDirection = { 0.0, -1.0, 0.25 };
}

void RasterizationStage::process()
{
    if (viewport->hasChanged())
        resizeTextures(viewport->width(), viewport->height());

    render();
}

void RasterizationStage::resizeTextures(int width, int height)
{
    diffuseBuffer->image2D(0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    specularBuffer->image2D(0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    faceNormalBuffer->image2D(0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    normalBuffer->image2D(0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    worldPosBuffer->image2D(0, GL_RGBA32F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
    depthBuffer->image2D(0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    
    worldPosBuffer->setParameter(GLenum::GL_TEXTURE_MAG_FILTER, GLenum::GL_NEAREST);
    worldPosBuffer->setParameter(GLenum::GL_TEXTURE_MIN_FILTER, GLenum::GL_NEAREST);

    m_fbo->printStatus(true);
}

void RasterizationStage::render()
{
    for (auto program : std::vector<globjects::Program*>{ m_program, m_shadowmap->program() })
    {
        program->setUniform("alpha", m_presetInformation.alpha);
    }

    auto lightRadius = m_presetInformation.lightMaxShift;

    auto frameLightOffset = m_kernelGenerationStage.shadowKernel[currentFrame - 1] * lightRadius;
    auto frameLightPosition = m_lightPosition + glm::vec3(frameLightOffset.x, 0.0f, frameLightOffset.y);

    auto biasedShadowTransform = m_shadowmap->render(frameLightPosition, m_lightDirection, m_modelLoadingStage.getDrawablesMap(), *m_groundPlane.get(), m_presetInformation.nearFar);

    glViewport(viewport->x(),
               viewport->y(),
               viewport->width(),
               viewport->height());

    m_fbo->bind();
    m_fbo->setDrawBuffers({
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3,
        GL_COLOR_ATTACHMENT4
    });

    auto maxFloat = std::numeric_limits<float>::max();

    m_fbo->clearBuffer(GL_COLOR, 0, glm::vec4(m_presetInformation.groundColor, 1.0f));
    m_fbo->clearBuffer(GL_COLOR, 1, glm::vec4(0.0f));
    m_fbo->clearBuffer(GL_COLOR, 2, glm::vec4(0.0f));
    m_fbo->clearBuffer(GL_COLOR, 3, glm::vec4(0.0f));
    m_fbo->clearBuffer(GL_COLOR, 4, glm::vec4(maxFloat));
    m_fbo->clearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);

    zPrepass();

    m_program->use();

    auto subpixelSample = m_kernelGenerationStage.antiAliasingKernel[currentFrame - 1];
    auto viewportSize = glm::vec2(viewport->width(), viewport->height());
    auto focalPoint = m_kernelGenerationStage.depthOfFieldKernel[currentFrame - 1] * m_presetInformation.focalPoint;
    focalPoint *= useDOF;

    for (auto program : std::vector<globjects::Program*>{ m_program, m_groundPlane->program() })
    {
        program->setUniform("shadowmap", ShadowSampler);
        program->setUniform("masksTexture", MaskSampler);
        program->setUniform("noiseTexture", NoiseSampler);
        program->setUniform("diffuseTexture", DiffuseSampler);
        program->setUniform("specularTexture", SpecularSampler);
        program->setUniform("emissiveTexture", EmissiveSampler);
        program->setUniform("opacityTexture", OpacitySampler);
        program->setUniform("bumpTexture", BumpSampler);

        program->setUniform("groundPlaneColor", m_presetInformation.groundColor);
        program->setUniform("worldLightPos", frameLightPosition);
        program->setUniform("biasedShadowTransform", biasedShadowTransform);

        program->setUniform("cameraEye", camera->eye());
        program->setUniform("modelView", camera->view());
        program->setUniform("projection", projection->projection());

        // offset needs to be doubled, because ndc range is [-1;1] and not [0;1]
        program->setUniform("ndcOffset", 2.0f * subpixelSample / viewportSize);

        program->setUniform("cocPoint", focalPoint);
        program->setUniform("focalDist", m_presetInformation.focalDist);
    }

    m_shadowmap->distanceTexture()->bindActive(ShadowSampler);

    for (auto& pair : m_modelLoadingStage.getDrawablesMap())
    {
        auto materialId = pair.first;
        auto& drawables = pair.second;

        auto& material = m_modelLoadingStage.getMaterialMap().at(materialId);

        bool hasDiffuseTex = material.hasTexture(TextureType::Diffuse);
        bool hasBumpTex = material.hasTexture(TextureType::Bump);
        bool hasSpecularTex = material.hasTexture(TextureType::Specular);
        bool hasEmissiveTex = material.hasTexture(TextureType::Emissive);
        bool hasOpacityTex = material.hasTexture(TextureType::Opacity);

        if (hasDiffuseTex)
        {
            auto tex = material.textureMap().at(TextureType::Diffuse);
            tex->bindActive(DiffuseSampler);
        }

        if (hasSpecularTex)
        {
            auto tex = material.textureMap().at(TextureType::Specular);
            tex->bindActive(SpecularSampler);
        }

        if (hasEmissiveTex)
        {
            auto tex = material.textureMap().at(TextureType::Emissive);
            tex->bindActive(EmissiveSampler);
        }

        if (hasOpacityTex)
        {
            auto tex = material.textureMap().at(TextureType::Opacity);
            tex->bindActive(OpacitySampler);
        }

        auto bumpType = BumpType::None;
        if (hasBumpTex)
        {
            bumpType = m_presetInformation.bumpType;
            auto tex = material.textureMap().at(TextureType::Bump);
            tex->bindActive(BumpSampler);
        }

        m_program->setUniform("shininess", material.specularFactor);

        m_program->setUniform("bumpType", static_cast<int>(bumpType));
        m_program->setUniform("useDiffuseTexture", hasDiffuseTex);
        m_program->setUniform("useSpecularTexture", hasSpecularTex);
        m_program->setUniform("useEmissiveTexture", hasEmissiveTex);
        m_program->setUniform("useOpacityTexture", hasOpacityTex);

        for (auto& drawable : drawables)
        {
            drawable->draw();
        }
    }

    m_program->release();

    m_groundPlane->draw();

    m_fbo->unbind();
}

void RasterizationStage::zPrepass()
{
    m_zOnlyProgram->use();

    for (auto& pair : m_modelLoadingStage.getDrawablesMap())
    {
        auto& drawables = pair.second;

        for (auto& drawable : drawables)
        {
            drawable->draw();
        }
    }

    m_zOnlyProgram->release();
}

void RasterizationStage::setupGLState()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}