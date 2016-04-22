#include "PostprocessingStage.h"

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

using namespace gl;

namespace
{
    const unsigned int s_ssaoKernelSize = 16;
    const unsigned int s_ssaoNoiseSize = 128;
}

PostprocessingStage::PostprocessingStage(KernelGenerationStage& kernelGenerationStage)
: m_kernelGenerationStage(kernelGenerationStage)
{
}

void PostprocessingStage::initialize()
{
    postprocessedFrame = globjects::Texture::createDefault(GL_TEXTURE_2D);

    m_fbo = new globjects::Framebuffer();
    m_fbo->attachTexture(GL_COLOR_ATTACHMENT0, postprocessedFrame);

    m_screenAlignedQuad = new gloperate::ScreenAlignedQuad(
        globjects::Shader::fromFile(GL_FRAGMENT_SHADER, "data/shaders/postprocessing.frag")
    );

    generateNoiseTexture();
    createKernelTexture();
}

void PostprocessingStage::process()
{
    const auto screenSize = glm::vec2(viewport->width(), viewport->height());

    if (viewport->hasChanged())
        resizeTexture(viewport->width(), viewport->height());

    updateKernelTexture();

    m_fbo->bind();
    m_fbo->setDrawBuffer(GL_COLOR_ATTACHMENT0);

    color->bindActive(0);
    normal->bindActive(1);
    depth->bindActive(2);
    m_ssaoKernelTexture->bindActive(3);
    m_ssaoNoiseTexture->bindActive(4);
    worldPos->bindActive(5);

    m_screenAlignedQuad->program()->setUniform("colorSampler", 0);
    m_screenAlignedQuad->program()->setUniform("normalSampler", 1);
    m_screenAlignedQuad->program()->setUniform("depthSampler", 2);
    m_screenAlignedQuad->program()->setUniform("ssaoKernelSampler", 3);
    m_screenAlignedQuad->program()->setUniform("ssaoNoiseSampler", 4);
    m_screenAlignedQuad->program()->setUniform("worldPosSampler", 5);

    m_screenAlignedQuad->program()->setUniform("ssaoRadius", presetInformation.lightMaxShift);
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

void PostprocessingStage::resizeTexture(int width, int height)
{
    postprocessedFrame->image2D(0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    m_fbo->printStatus(true);
}

void PostprocessingStage::generateNoiseTexture()
{
    auto& noise = m_kernelGenerationStage.getSSAONoise(s_ssaoNoiseSize);
    auto size = static_cast<int>(std::sqrt(noise.size()));

    auto texture = new globjects::Texture(gl::GL_TEXTURE_2D);
    texture->setParameter(gl::GL_TEXTURE_MIN_FILTER, gl::GL_NEAREST);
    texture->setParameter(gl::GL_TEXTURE_MAG_FILTER, gl::GL_NEAREST);
    texture->setParameter(gl::GL_TEXTURE_WRAP_S, gl::GL_MIRRORED_REPEAT);
    texture->setParameter(gl::GL_TEXTURE_WRAP_T, gl::GL_MIRRORED_REPEAT);

    texture->image2D(0, gl::GL_RGBA32F, glm::ivec2(size), 0, gl::GL_RGB, gl::GL_FLOAT, noise.data());

    m_ssaoNoiseTexture = texture;
}

void PostprocessingStage::createKernelTexture()
{
    auto texture = new globjects::Texture(gl::GL_TEXTURE_1D);
    texture->setParameter(gl::GL_TEXTURE_MIN_FILTER, gl::GL_NEAREST);
    texture->setParameter(gl::GL_TEXTURE_MAG_FILTER, gl::GL_NEAREST);
    texture->setParameter(gl::GL_TEXTURE_WRAP_S, gl::GL_MIRRORED_REPEAT);

    m_ssaoKernelTexture = texture;
}

void PostprocessingStage::updateKernelTexture()
{
    auto kernel = m_kernelGenerationStage.getSSAOKernel(s_ssaoKernelSize);

    m_ssaoKernelTexture->image1D(0, gl::GL_RGBA32F, kernel.size(), 0, gl::GL_RGB, gl::GL_FLOAT, kernel.data());
}

