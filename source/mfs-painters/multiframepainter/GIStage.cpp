#include "GIStage.h"

#include <memory>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>

#include <globjects/Texture.h>
#include <globjects/Program.h>
#include <globjects/Framebuffer.h>
#include <globjects/Shader.h>

#include <gloperate/painter/CameraCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/ViewportCapability.h>
#include <gloperate/primitives/ScreenAlignedQuad.h>

#include "ModelLoadingStage.h"

using namespace gl;

GIStage::GIStage(ModelLoadingStage& modelLoadingStage, KernelGenerationStage& kernelGenerationStage)
: modelLoadingStage(modelLoadingStage)
{
    m_rsmRenderer = std::make_unique<RasterizationStage>(modelLoadingStage, kernelGenerationStage);
    m_lightCamera = std::make_unique<gloperate::CameraCapability>();
    m_lightViewport = std::make_unique<gloperate::ViewportCapability>();
    m_lightProjection = std::make_unique<gloperate::PerspectiveProjectionCapability>(m_lightViewport.get());
}

GIStage::~GIStage()
{

}


void GIStage::initialize()
{
    giBuffer = globjects::Texture::createDefault(GL_TEXTURE_2D);

    m_fbo = new globjects::Framebuffer();
    m_fbo->attachTexture(GL_COLOR_ATTACHMENT0, giBuffer);


    m_program = new globjects::Program();
    m_program->attach(
        globjects::Shader::fromFile(GL_VERTEX_SHADER, "data/shaders/deferredshading.vert"),
        globjects::Shader::fromFile(GL_FRAGMENT_SHADER, "data/shaders/deferredshading.frag"));

    m_screenAlignedQuad = new gloperate::ScreenAlignedQuad(m_program);

    m_lightPosition = { 450.0, 1870.0, -250.0 };
    m_lightDirection = { 0.0, -1.0, 0.25 };

    m_lightCamera->setEye(m_lightPosition);
    m_lightCamera->setCenter(m_lightPosition + m_lightDirection);
    m_rsmRenderer->camera = m_lightCamera.get();

    m_lightViewport->setViewport(0, 0, 512, 512);
    m_rsmRenderer->viewport = m_lightViewport.get();

    m_lightProjection->setFovy(90);
    m_lightProjection->setZFar(5000);
    m_lightProjection->setZNear(5);
    m_rsmRenderer->projection = m_lightProjection.get();

    m_rsmRenderer->initialize();
}

void GIStage::process()
{
    m_rsmRenderer->process();


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
    m_rsmRenderer->diffuseBuffer->bindActive(2);
    m_rsmRenderer->faceNormalBuffer->bindActive(3);
    m_rsmRenderer->depthBuffer->bindActive(4);


    m_screenAlignedQuad->program()->setUniform("diffuseSampler", 0);
    m_screenAlignedQuad->program()->setUniform("specularSampler", 1);
    m_screenAlignedQuad->program()->setUniform("faceNormalSampler", 2);
    m_screenAlignedQuad->program()->setUniform("normalSampler", 3);
    m_screenAlignedQuad->program()->setUniform("depthSampler", 4);
    m_screenAlignedQuad->program()->setUniform("shadowmap", 5);

    m_screenAlignedQuad->program()->setUniform("projectionMatrix", projection->projection());
    m_screenAlignedQuad->program()->setUniform("projectionInverseMatrix", projection->projectionInverted());
    //m_screenAlignedQuad->program()->setUniform("normalMatrix", camera->normal());
    //m_screenAlignedQuad->program()->setUniform("viewMatrix", camera->view());
    m_screenAlignedQuad->program()->setUniform("viewInvertedMatrix", camera->viewInverted());
    m_screenAlignedQuad->program()->setUniform("worldLightPos", m_lightPosition);
    //m_screenAlignedQuad->program()->setUniform("biasedLightViewProjectionMatrix", biasedShadowTransform);
    m_screenAlignedQuad->program()->setUniform("cameraEye", camera->eye());
    m_screenAlignedQuad->program()->setUniform("zFar", projection->zFar());
    m_screenAlignedQuad->program()->setUniform("zNear", projection->zNear());
    m_screenAlignedQuad->program()->setUniform("screenSize", screenSize);

    m_screenAlignedQuad->draw();

    m_fbo->unbind();
}

void GIStage::resizeTexture(int width, int height)
{
    giBuffer->image2D(0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
    m_fbo->printStatus(true);
}

