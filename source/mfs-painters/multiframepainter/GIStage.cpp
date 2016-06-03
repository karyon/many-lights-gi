#include "GIStage.h"

#include <memory>

#include <glm/gtc/matrix_transform.hpp>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>

#include <globjects/Texture.h>
#include <globjects/Program.h>
#include <globjects/Framebuffer.h>
#include <globjects/Shader.h>

#include <gloperate/painter/CameraCapability.h>
#include <gloperate/painter/OrthographicProjectionCapability.h>
#include <gloperate/painter/ViewportCapability.h>
#include <gloperate/primitives/ScreenAlignedQuad.h>

#include <reflectionzeug/property/extensions/GlmProperties.h>

#include "ModelLoadingStage.h"
#include "MultiFramePainter.h"
#include "PerfCounter.h"
#include "Shadowmap.h"
#include "ImperfectShadowmap.h"
#include "VPLProcessor.h"

using namespace gl;

GIStage::GIStage(ModelLoadingStage& modelLoadingStage, KernelGenerationStage& kernelGenerationStage)
: modelLoadingStage(modelLoadingStage)
{
    rsmRenderer = std::make_unique<RasterizationStage>("RSM", modelLoadingStage, kernelGenerationStage);
    m_lightCamera = std::make_unique<gloperate::CameraCapability>();
    m_lightViewport = std::make_unique<gloperate::ViewportCapability>();
    m_lightProjection = std::make_unique<gloperate::OrthographicProjectionCapability>(m_lightViewport.get());
}

GIStage::~GIStage()
{

}

void GIStage::initProperties(MultiFramePainter& painter)
{
    painter.addProperty<glm::vec3>("RSMLightPosition",
        [this]() { return lightPosition; },
        [this](const glm::vec3 & pos) {
            lightPosition = pos;
        });
    painter.addProperty<glm::vec3>("RSMLightDirection",
        [this]() { return lightDirection; },
        [this](const glm::vec3 & dir) {
            lightDirection = dir;
        });

    painter.addProperty<float>("LightIntensity",
        [this]() { return lightIntensity; },
        [this](const float & intensity) {
            lightIntensity = intensity;
        }
    )->setOptions({
        { "minimum", 0.0f },
        { "step", 0.2f },
        { "precision", 2u },
    });

    painter.addProperty<float>("GIIntensityFactor",
        [this]() { return giIntensityFactor; },
        [this](const float & factor) {
            giIntensityFactor = factor;
        }
    )->setOptions({
        { "minimum", 0.0f },
        { "step", 100.0f },
        { "precision", 1u },
    });

    painter.addProperty<float>("VPLClampingValue",
        [this]() { return vplClampingValue; },
        [this](const float & value) {
            vplClampingValue = value;
        }
    )->setOptions({
        { "minimum", 0.0f },
        { "step", 0.0001f },
        { "precision", 5u },
    });

    painter.addProperty<int>("VPLStartIndex",
	    [this]() { return vplStartIndex; },
	    [this](const int & value) {
	    vplStartIndex = value;
	    }
    )->setOptions({
	    { "minimum", 0 },
	    { "maximum", 256}
    });

    painter.addProperty<int>("VPLEndIndex",
        [this]() { return vplEndIndex; },
        [this](const int & value) {
            vplEndIndex = value;
        }
    )->setOptions({
        { "minimum", 0 },
        { "maximum", 256 }
    });

    painter.addProperty<bool>("ScaleISMs",
        [this]() { return scaleISMs; },
        [this](const bool & value) {
            scaleISMs = value;
    });

    painter.addProperty<bool>("PointsOnlyToScaledISMs",
        [this]() { return pointsOnlyIntoScaledISMs; },
        [this](const bool & value) {
            pointsOnlyIntoScaledISMs = value;
    });

    painter.addProperty<float>("TessLevelFactor",
        [this]() { return tessLevelFactor; },
        [this](const float & value) {
            tessLevelFactor = value;
        }
    )->setOptions({
        { "minimum", 0.0f },
        { "step", 0.05f },
        { "precision", 3u },
    });

    painter.addProperty<bool>("ShowLightPositions",
        [this]() { return showLightPositions; },
        [this](const bool & value) {
            showLightPositions = value;
    });
}

void GIStage::initialize()
{
    giBuffer = globjects::Texture::createDefault(GL_TEXTURE_2D);
    giBuffer->setName("GI Buffer");

    m_fbo = new globjects::Framebuffer();
    m_fbo->attachTexture(GL_COLOR_ATTACHMENT0, giBuffer);


    m_program = new globjects::Program();
    m_program->attach(
        globjects::Shader::fromFile(GL_VERTEX_SHADER, "data/shaders/deferredshading.vert"),
        globjects::Shader::fromFile(GL_FRAGMENT_SHADER, "data/shaders/gi.frag"));

    m_screenAlignedQuad = new gloperate::ScreenAlignedQuad(m_program);


    lightPosition = { -0.5, 18.70, -2.50 };
    lightDirection = { 0.0, -1.0, 0.25 };
    lightIntensity = 5.0f;

    giIntensityFactor = 3000.0f;
    vplClampingValue = 0.001f;
    vplStartIndex = 0;
    vplEndIndex = 256;
    scaleISMs = false;
    pointsOnlyIntoScaledISMs = false;
    tessLevelFactor = 2.0f;
    showLightPositions = false;

    rsmRenderer->camera = m_lightCamera.get();

    m_lightViewport->setViewport(0, 0, 512, 128);
    rsmRenderer->viewport = m_lightViewport.get();
    m_lightProjection->setHeight(5);

    m_lightProjection->setZFar(50);
    m_lightProjection->setZNear(0.05);

    rsmRenderer->projection = m_lightProjection.get();

    rsmRenderer->initialize();

    shadowmap = std::make_unique<Shadowmap>();
    ism = std::make_unique<ImperfectShadowmap>();
    vplProcessor = std::make_unique<VPLProcessor>();
}

void GIStage::process()
{
    m_lightCamera->setEye(lightPosition);
    m_lightCamera->setCenter(lightPosition + lightDirection);

    auto view = rsmRenderer->camera->view();
    auto viewProjection = rsmRenderer->projection->projection() * view;
    auto nearFar = glm::vec2(projection->zNear(), projection->zFar());

    {
        AutoGLPerfCounter c("Shadowmap");
        shadowmap->render(lightPosition, viewProjection, modelLoadingStage.getDrawablesMap(), nearFar);
    }

    {
        AutoGLPerfCounter c("RSM");
        rsmRenderer->process();
    }

    {
        AutoGLPerfCounter c("VPL processing");
        vplProcessor->process(*rsmRenderer.get(), lightIntensity);
    }

    {
        AutoGLPerfCounter c("ISM");
        ism->render(modelLoadingStage.getDrawablesMap(), *vplProcessor.get(), vplStartIndex, vplEndIndex, scaleISMs, pointsOnlyIntoScaledISMs, tessLevelFactor, m_lightProjection->zFar());
    }

    AutoGLPerfCounter c("GI");


    gl::glViewport(viewport->x(),
        viewport->y(),
        viewport->width(),
        viewport->height());

    const auto screenSize = glm::vec2(viewport->width(), viewport->height());

    if (viewport->hasChanged())
        resizeTexture(viewport->width(), viewport->height());

    m_fbo->bind();
    m_fbo->setDrawBuffer(GL_COLOR_ATTACHMENT0);

    faceNormalBuffer->bindActive(0);
    depthBuffer->bindActive(1);
    ism->depthBuffer->bindActive(2);


    vplProcessor->vplBuffer->bindBase(GL_UNIFORM_BUFFER, 0);


    m_screenAlignedQuad->program()->setUniform("faceNormalSampler", 0);
    m_screenAlignedQuad->program()->setUniform("depthSampler", 1);
    m_screenAlignedQuad->program()->setUniform("ismDepthSampler", 2);

    m_screenAlignedQuad->program()->setUniform("projectionMatrix", projection->projection());
    m_screenAlignedQuad->program()->setUniform("projectionInverseMatrix", projection->projectionInverted());
    m_screenAlignedQuad->program()->setUniform("viewMatrix", camera->view());
    m_screenAlignedQuad->program()->setUniform("viewInvertedMatrix", camera->viewInverted());
    m_screenAlignedQuad->program()->setUniform("zFar", projection->zFar());
    m_screenAlignedQuad->program()->setUniform("zNear", projection->zNear());
    m_screenAlignedQuad->program()->setUniform("giIntensityFactor", giIntensityFactor);
    m_screenAlignedQuad->program()->setUniform("vplClampingValue", vplClampingValue);
    m_screenAlignedQuad->program()->setUniform("vplStartIndex", vplStartIndex);
    m_screenAlignedQuad->program()->setUniform("vplEndIndex", vplEndIndex);
    m_screenAlignedQuad->program()->setUniform("scaleISMs", scaleISMs);
    m_screenAlignedQuad->program()->setUniform("showLightPositions", showLightPositions);

    m_screenAlignedQuad->draw();

    m_fbo->unbind();
}

void GIStage::resizeTexture(int width, int height)
{
    giBuffer->image2D(0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
    m_fbo->printStatus(true);
}

