#include "SSAOStage.h"

#include <glbinding/gl/enum.h>

#include <glkernel/Kernel.h>

#include <globjects/Texture.h>
#include <globjects/Program.h>
#include <globjects/Framebuffer.h>

#include <gloperate/primitives/ScreenAlignedQuad.h>
#include <gloperate/painter/AbstractViewportCapability.h>
#include <gloperate/painter/AbstractPerspectiveProjectionCapability.h>
#include <gloperate/painter/AbstractCameraCapability.h>

#include "KernelGenerationStage.h"
#include "PerfCounter.h"

using namespace gl;

namespace
{
    const unsigned int s_ssaoKernelSize = 16;
    const unsigned int s_ssaoNoiseSize = 128;
}

SSAOStage::SSAOStage(KernelGenerationStage& kernelGenerationStage, const PresetInformation& presetInformation)
: m_kernelGenerationStage(kernelGenerationStage)
, m_presetInformation(presetInformation)
{
}

void SSAOStage::initialize()
{
    occlusionBuffer = globjects::Texture::createDefault(GL_TEXTURE_2D);
    occlusionBuffer->setName("Occlusion");

    m_fbo = new globjects::Framebuffer();
    m_fbo->attachTexture(GL_COLOR_ATTACHMENT0, occlusionBuffer);

    m_screenAlignedQuad = new gloperate::ScreenAlignedQuad(
        globjects::Shader::fromFile(GL_FRAGMENT_SHADER, "data/shaders/ssao.frag")
    );

    generateNoiseTexture();
    createKernelTexture();
}

void SSAOStage::process()
{
    AutoGLPerfCounter c("SSAO");

    const auto screenSize = glm::vec2(viewport->width(), viewport->height());

    if (viewport->hasChanged())
        resizeTexture(viewport->width(), viewport->height());

    //updateKernelTexture();

    m_fbo->bind();
    m_fbo->setDrawBuffer(GL_COLOR_ATTACHMENT0);

    faceNormalBuffer->bindActive(0);
    depthBuffer->bindActive(1);
    m_ssaoKernelTexture->bindActive(2);
    m_ssaoNoiseTexture->bindActive(3);

    m_screenAlignedQuad->program()->setUniform("normalSampler", 0);
    m_screenAlignedQuad->program()->setUniform("depthSampler", 1);
    m_screenAlignedQuad->program()->setUniform("ssaoKernelSampler", 2);
    m_screenAlignedQuad->program()->setUniform("ssaoNoiseSampler", 3);

    m_screenAlignedQuad->program()->setUniform("ssaoRadius", m_presetInformation.lightMaxShift);
    m_screenAlignedQuad->program()->setUniform("projectionMatrix", projection->projection());
    m_screenAlignedQuad->program()->setUniform("projectionInverseMatrix", projection->projectionInverted());
    m_screenAlignedQuad->program()->setUniform("normalMatrix", camera->normal());
    m_screenAlignedQuad->program()->setUniform("view", camera->view());
    m_screenAlignedQuad->program()->setUniform("farZ", projection->zFar());
    m_screenAlignedQuad->program()->setUniform("screenSize", screenSize);
    m_screenAlignedQuad->program()->setUniform("samplerSizes", glm::vec4(s_ssaoKernelSize, 1.f / s_ssaoKernelSize, s_ssaoNoiseSize, 1.f / s_ssaoNoiseSize));
 

    m_screenAlignedQuad->draw();

    m_fbo->unbind();
}

void SSAOStage::resizeTexture(int width, int height)
{
    occlusionBuffer->image2D(0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    m_fbo->printStatus(true);
}

void SSAOStage::generateNoiseTexture()
{
    auto& noise = m_kernelGenerationStage.getSSAONoise(s_ssaoNoiseSize);
    auto size = static_cast<int>(std::sqrt(noise.size()));

    auto texture = new globjects::Texture(gl::GL_TEXTURE_2D);
    texture->setParameter(gl::GL_TEXTURE_MIN_FILTER, gl::GL_NEAREST);
    texture->setParameter(gl::GL_TEXTURE_MAG_FILTER, gl::GL_NEAREST);
    texture->setParameter(gl::GL_TEXTURE_WRAP_S, gl::GL_REPEAT);
    texture->setParameter(gl::GL_TEXTURE_WRAP_T, gl::GL_REPEAT);

    texture->image2D(0, gl::GL_RGBA32F, glm::ivec2(size), 0, gl::GL_RGB, gl::GL_FLOAT, noise.data());

    m_ssaoNoiseTexture = texture;
}

void SSAOStage::createKernelTexture()
{
    auto texture = new globjects::Texture(gl::GL_TEXTURE_1D);
    texture->setParameter(gl::GL_TEXTURE_MIN_FILTER, gl::GL_NEAREST);
    texture->setParameter(gl::GL_TEXTURE_MAG_FILTER, gl::GL_NEAREST);
    texture->setParameter(gl::GL_TEXTURE_WRAP_S, gl::GL_REPEAT);

    m_ssaoKernelTexture = texture;
    updateKernelTexture();
}

void SSAOStage::updateKernelTexture()
{
    auto kernel = m_kernelGenerationStage.getSSAOKernel(s_ssaoKernelSize);

    m_ssaoKernelTexture->image1D(0, gl::GL_RGBA32F, kernel.size(), 0, gl::GL_RGB, gl::GL_FLOAT, kernel.data());
}

