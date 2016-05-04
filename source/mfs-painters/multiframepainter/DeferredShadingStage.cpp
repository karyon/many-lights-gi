#include "DeferredShadingStage.h"

#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>

#include <glkernel/Kernel.h>

#include <globjects/Texture.h>
#include <globjects/Program.h>
#include <globjects/Framebuffer.h>

#include <gloperate/primitives/ScreenAlignedQuad.h>
#include <gloperate/painter/AbstractViewportCapability.h>
#include <gloperate/painter/AbstractPerspectiveProjectionCapability.h>
#include <gloperate/painter/AbstractCameraCapability.h>

#include <reflectionzeug/property/extensions/GlmProperties.h>

#include "KernelGenerationStage.h"
#include "ModelLoadingStage.h"
#include "MultiFramePainter.h"
#include "Shadowmap.h"
#include "PerfCounter.h"

using namespace gl;

namespace
{
    const unsigned int s_ssaoKernelSize = 16;
    const unsigned int s_ssaoNoiseSize = 128;
}

DeferredShadingStage::DeferredShadingStage(ModelLoadingStage& modelLoadingStage)
: m_modelLoadingStage(modelLoadingStage)
{
}

DeferredShadingStage::~DeferredShadingStage()
{
}

void DeferredShadingStage::initProperties(MultiFramePainter& painter)
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

void DeferredShadingStage::initialize()
{
    shadedFrame = globjects::Texture::createDefault(GL_TEXTURE_2D);

    m_fbo = new globjects::Framebuffer();
    m_fbo->attachTexture(GL_COLOR_ATTACHMENT0, shadedFrame);


    m_program = new globjects::Program();
    m_program->attach(
        globjects::Shader::fromFile(GL_VERTEX_SHADER, "data/shaders/deferredshading.vert"),
        globjects::Shader::fromFile(GL_FRAGMENT_SHADER, "data/shaders/deferredshading.frag"));

    m_screenAlignedQuad = new gloperate::ScreenAlignedQuad(m_program);

    m_shadowmap = std::make_unique<Shadowmap>();

    m_lightPosition = { 450.0, 1870.0, -250.0 };
    m_lightDirection = { 0.0, -1.0, 0.25 };

}

void DeferredShadingStage::process()
{
    AutoGLPerfCounter c("Deferred Shading");

    auto nearFar = glm::vec2(projection->zNear(), projection->zFar());
    auto biasedShadowTransform = m_shadowmap->render(m_lightPosition, m_lightDirection, m_modelLoadingStage.getDrawablesMap(), nearFar);


    gl::glViewport(viewport->x(),
        viewport->y(),
        viewport->width(),
        viewport->height());

    const auto screenSize = glm::vec2(viewport->width(), viewport->height());

    if (viewport->hasChanged())
        resizeTexture(viewport->width(), viewport->height());

    m_fbo->bind();
    m_fbo->setDrawBuffer(GL_COLOR_ATTACHMENT0);

    diffuseBuffer->bindActive(0);
    specularBuffer->bindActive(1);
    faceNormalBuffer->bindActive(2);
    normalBuffer->bindActive(3);
    depthBuffer->bindActive(4);
    m_shadowmap->distanceTexture()->bindActive(5);
    giBuffer->bindActive(6);


    m_screenAlignedQuad->program()->setUniform("diffuseSampler", 0);
    m_screenAlignedQuad->program()->setUniform("specularSampler", 1);
    m_screenAlignedQuad->program()->setUniform("faceNormalSampler", 2);
    m_screenAlignedQuad->program()->setUniform("normalSampler", 3);
    m_screenAlignedQuad->program()->setUniform("depthSampler", 4);
    m_screenAlignedQuad->program()->setUniform("shadowmap", 5);
    m_screenAlignedQuad->program()->setUniform("giSampler", 6);

    m_screenAlignedQuad->program()->setUniform("projectionMatrix", projection->projection());
    m_screenAlignedQuad->program()->setUniform("projectionInverseMatrix", projection->projectionInverted());
    m_screenAlignedQuad->program()->setUniform("viewMatrix", camera->view());
    m_screenAlignedQuad->program()->setUniform("viewInvertedMatrix", camera->viewInverted());
    m_screenAlignedQuad->program()->setUniform("worldLightPos", m_lightPosition);
    m_screenAlignedQuad->program()->setUniform("biasedLightViewProjectionMatrix", biasedShadowTransform);
    m_screenAlignedQuad->program()->setUniform("cameraEye", camera->eye());
    m_screenAlignedQuad->program()->setUniform("zFar", projection->zFar());
    m_screenAlignedQuad->program()->setUniform("zNear", projection->zNear());
    m_screenAlignedQuad->program()->setUniform("screenSize", screenSize);

    m_screenAlignedQuad->draw();

    m_fbo->unbind();
}

void DeferredShadingStage::resizeTexture(int width, int height)
{
    shadedFrame->image2D(0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
    m_fbo->printStatus(true);
}
