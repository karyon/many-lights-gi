#include "GIStage.h"

#include <memory>

#include <glm/gtc/matrix_transform.hpp>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>
#include <glbinding/gl/boolean.h>

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
#include "ClusteredShading.h"
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
    //painter.addProperty<glm::vec3>("RSMLightPosition",
    //    [this]() { return m_lightCamera->eye(); },
    //    [this](const glm::vec3 & pos) {
    //        m_lightCamera->setEye(pos);
    //    });
    //painter.addProperty<glm::vec3>("RSMLightDirection",
    //    [this]() { return m_lightCamera->center() - m_lightCamera->eye(); },
    //    [this](const glm::vec3 & dir) {
    //        m_lightCamera->setCenter(m_lightCamera->eye() + dir);
    //    });
    painter.addProperty<glm::vec3>("RSMLightCenter",
        [this]() { return m_lightCamera->center(); },
        [this](const glm::vec3 & center) {
            m_lightCamera->setCenter(center);
        });

    painter.addProperty<bool>("MoveSun",
        [this]() { return moveLight; },
        [this](const bool & value) {
        moveLight = value;
    });

    painter.addProperty<float>("SunCyclePosition",
        [this]() { return sunCyclePosition; },
        [this](const float & value) {
            sunCyclePosition = value;
        });
    painter.addProperty<float>("SunCycleSpeed",
        [this]() { return sunCycleSpeed; },
        [this](const float & value) {
            sunCycleSpeed = value;
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
        { "maximum", 1024}
    });

    painter.addProperty<int>("VPLEndIndex",
        [this]() { return vplEndIndex; },
        [this](const int & value) {
            vplEndIndex = value;
        }
    )->setOptions({
        { "minimum", 0 },
        { "maximum", 1024 }
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

    painter.addProperty<bool>("GIShadowing",
        [this]() { return enableShadowing; },
        [this](const bool & value) {
            enableShadowing = value;
    });

    painter.addProperty<bool>("ShowVPLPositions",
        [this]() { return showVPLPositions; },
        [this](const bool & value) {
            showVPLPositions = value;
    });
}

void GIStage::initialize()
{
    giBuffer = globjects::Texture::createDefault(GL_TEXTURE_2D);
    giBuffer->setName("GI Buffer");

    m_fbo = new globjects::Framebuffer();
    m_fbo->attachTexture(GL_COLOR_ATTACHMENT0, giBuffer);

    auto program = new globjects::Program();
    program->attach(
        globjects::Shader::fromFile(GL_COMPUTE_SHADER, "data/shaders/gi.comp")
        //globjects::Shader::fromFile(GL_VERTEX_SHADER, "data/shaders/deferredshading.vert"),
        //globjects::Shader::fromFile(GL_FRAGMENT_SHADER, "data/shaders/gi.frag")
    );

    m_screenAlignedQuad = new gloperate::ScreenAlignedQuad(program);


    giBlurTempBuffer = globjects::Texture::createDefault(GL_TEXTURE_2D);
    giBlurTempBuffer->setName("GI Temp Buffer");

    giBlurFinalBuffer = globjects::Texture::createDefault(GL_TEXTURE_2D);
    giBlurFinalBuffer->setName("GI Final Buffer");

    m_blurTempFbo = new globjects::Framebuffer();
    m_blurTempFbo->attachTexture(GL_COLOR_ATTACHMENT0, giBlurTempBuffer);

    m_blurFinalFbo = new globjects::Framebuffer();
    m_blurFinalFbo->attachTexture(GL_COLOR_ATTACHMENT0, giBlurFinalBuffer);

    globjects::Shader::globalReplace("#define DIRECTION ivec2(0,0)", "#define DIRECTION ivec2(1,0)");
    auto blurFragShaderX = globjects::Shader::fromFile(GL_FRAGMENT_SHADER, "data/shaders/gi_blur.frag");

    globjects::Shader::clearGlobalReplacements();
    globjects::Shader::globalReplace("#define DIRECTION ivec2(0,0)", "#define DIRECTION ivec2(0,1)");
    auto blurFragShaderY = globjects::Shader::fromFile(GL_FRAGMENT_SHADER, "data/shaders/gi_blur.frag");

    auto blurXProgram = new globjects::Program();
    blurXProgram->attach(
        globjects::Shader::fromFile(GL_VERTEX_SHADER, "data/shaders/deferredshading.vert"),
        blurFragShaderX);

    auto blurYProgram = new globjects::Program();
    blurYProgram->attach(
        globjects::Shader::fromFile(GL_VERTEX_SHADER, "data/shaders/deferredshading.vert"),
        blurFragShaderY);

    m_blurXScreenAlignedQuad = new gloperate::ScreenAlignedQuad(blurXProgram);
    m_blurYScreenAlignedQuad = new gloperate::ScreenAlignedQuad(blurYProgram);

    m_lightCamera->setCenter(glm::vec3(-0.5, 13.0, 0.0));
    lightIntensity = 5.0f;

    giIntensityFactor = 3000.0f;
    vplClampingValue = 0.001f;
    vplStartIndex = 0;
    vplEndIndex = 1024;
    scaleISMs = false;
    pointsOnlyIntoScaledISMs = false;
    tessLevelFactor = 2.0f;
    enableShadowing = true;
    showVPLPositions = false;
    moveLight = false;
    sunCyclePosition = 266.0f;
    sunCycleSpeed = 0.1f;

    rsmRenderer->camera = m_lightCamera.get();

    m_lightViewport->setViewport(0, 0, 512, 128);
    rsmRenderer->viewport = m_lightViewport.get();
    m_lightProjection->setHeight(5);

    m_lightProjection->setZFar(50);
    m_lightProjection->setZNear(1.0f);

    rsmRenderer->projection = m_lightProjection.get();

    rsmRenderer->initialize();

    shadowmap = std::make_unique<Shadowmap>();
    ism = std::make_unique<ImperfectShadowmap>();
    vplProcessor = std::make_unique<VPLProcessor>();
    clusteredShading = std::make_unique<ClusteredShading>();
}

void GIStage::render()
{
    lightPosition = m_lightCamera->eye();
    lightDirection = m_lightCamera->center() - m_lightCamera->eye();

    giBuffer->bindImageTexture(0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R11F_G11F_B10F);
    clusteredShading->clusterIDs->bindImageTexture(1, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8UI);
    clusteredShading->normalToCompactIDs->bindImageTexture(2, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16UI);
    clusteredShading->lightLists->bindImageTexture(3, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16UI);

    faceNormalBuffer->bindActive(0);
    depthBuffer->bindActive(1);
    ism->depthBuffer->bindActive(2);


    vplProcessor->vplBuffer->bindBase(GL_UNIFORM_BUFFER, 0);


    m_screenAlignedQuad->program()->setUniform("faceNormalSampler", 0);
    m_screenAlignedQuad->program()->setUniform("depthSampler", 1);
    m_screenAlignedQuad->program()->setUniform("ismDepthSampler", 2);

    m_screenAlignedQuad->program()->setUniform("projectionMatrix", projection->projection());
    m_screenAlignedQuad->program()->setUniform("projectionInverseMatrix", projection->projectionInverted());
    m_screenAlignedQuad->program()->setUniform("viewport", glm::ivec2(viewport->width(), viewport->height()));
    m_screenAlignedQuad->program()->setUniform("viewMatrix", camera->view());
    m_screenAlignedQuad->program()->setUniform("viewInvertedMatrix", camera->viewInverted());
    m_screenAlignedQuad->program()->setUniform("viewProjectionInvertedMatrix", camera->viewInverted() * projection->projectionInverted());
    m_screenAlignedQuad->program()->setUniform("zFar", projection->zFar());
    m_screenAlignedQuad->program()->setUniform("zNear", projection->zNear());
    m_screenAlignedQuad->program()->setUniform("giIntensityFactor", giIntensityFactor);
    m_screenAlignedQuad->program()->setUniform("vplClampingValue", vplClampingValue);
    m_screenAlignedQuad->program()->setUniform("vplStartIndex", vplStartIndex);
    m_screenAlignedQuad->program()->setUniform("vplEndIndex", vplEndIndex);
    m_screenAlignedQuad->program()->setUniform("scaleISMs", scaleISMs);
    m_screenAlignedQuad->program()->setUniform("enableShadowing", enableShadowing);
    m_screenAlignedQuad->program()->setUniform("showVPLPositions", showVPLPositions);

    //m_screenAlignedQuad->draw();

    m_screenAlignedQuad->program()->dispatchCompute(viewport->width() / 4 / 8 + 1, viewport->height() / 4 / 8 + 1, 16);
    giBuffer->unbindImageTexture(0);
}

void GIStage::blur()
{
    giBuffer->bindActive(0); 
    faceNormalBuffer->bindActive(1);
    depthBuffer->bindActive(2);

    m_blurTempFbo->bind();
    m_blurTempFbo->setDrawBuffer(GL_COLOR_ATTACHMENT0);

    m_blurXScreenAlignedQuad->program()->setUniform("giSampler", 0);
    m_blurXScreenAlignedQuad->program()->setUniform("faceNormalSampler", 1);
    m_blurXScreenAlignedQuad->program()->setUniform("depthSampler", 2);

    m_blurXScreenAlignedQuad->program()->setUniform("projectionMatrix", projection->projection());
    m_blurXScreenAlignedQuad->program()->setUniform("projectionInverseMatrix", projection->projectionInverted());

    m_blurXScreenAlignedQuad->draw();

    m_blurTempFbo->unbind();


    giBlurTempBuffer->bindActive(0);

    m_blurFinalFbo->bind();
    m_blurFinalFbo->setDrawBuffer(GL_COLOR_ATTACHMENT0);

    m_blurYScreenAlignedQuad->program()->setUniform("giSampler", 0);
    m_blurYScreenAlignedQuad->program()->setUniform("faceNormalSampler", 1);
    m_blurYScreenAlignedQuad->program()->setUniform("depthSampler", 2);

    m_blurYScreenAlignedQuad->program()->setUniform("projectionMatrix", projection->projection());
    m_blurYScreenAlignedQuad->program()->setUniform("projectionInverseMatrix", projection->projectionInverted());

    m_blurYScreenAlignedQuad->draw();

    m_blurFinalFbo->unbind();
}

void GIStage::process()
{
    if (viewport->hasChanged()) {
        resizeTexture(viewport->width(), viewport->height());
    }

    const float degreeSpan = 80.0f;
    float degree = glm::abs(glm::mod(sunCyclePosition, degreeSpan*2) - degreeSpan) + (180.0-degreeSpan)/2;
    float radians = glm::radians(degree);
    glm::vec3 direction = { 0.0, -glm::sin(radians), glm::cos(radians) };
    m_lightCamera->setEye(m_lightCamera->center() - direction);
    if (moveLight) {
        sunCyclePosition += sunCycleSpeed;
        sunCyclePosition = glm::mod(sunCyclePosition, degreeSpan * 2);
    }

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

    gl::glViewport(viewport->x(),
        viewport->y(),
        viewport->width(),
        viewport->height());

    {
        clusteredShading->process(*vplProcessor.get(), projection->projection(), depthBuffer, glm::ivec2(viewport->width(), viewport->height()));
    }

    {
        AutoGLPerfCounter c("GI");
        render();
    }

    {
        AutoGLPerfCounter c("GI blur");
        blur();
    }
}

void GIStage::resizeTexture(int width, int height)
{
    giBuffer->image2D(0, GL_R11F_G11F_B10F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    giBlurTempBuffer->image2D(0, GL_R11F_G11F_B10F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
    giBlurFinalBuffer->image2D(0, GL_R11F_G11F_B10F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
    m_fbo->printStatus(true);
    clusteredShading->resizeTexture(width, height);
}

