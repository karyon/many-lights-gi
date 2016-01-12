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

using namespace gl;

PostprocessingStage::PostprocessingStage()
{
    addInput("projection", projection);
    addInput("viewport", viewport);
    addInput("camera", camera);
    addInput("color", color);
    addInput("normal", normal);
    addInput("worldPos", worldPos);
    addInput("reflectMask", reflectMask);
    addInput("presetInformation", presetInformation);
    addInput("useReflections", useReflections);
    addInput("reflectionKernel", reflectionKernel);
    addInput("ssaoKernel", ssaoKernel);
    addInput("ssaoNoise", ssaoNoise);
    addInput("ssaoKernelSize", ssaoKernelSize);
    addInput("ssaoNoiseSize", ssaoNoiseSize);
    addInput("currentFrame", currentFrame);

    addOutput("postprocessedFrame", postprocessedFrame);
}

void PostprocessingStage::initialize()
{
    postprocessedFrame.data() = globjects::Texture::createDefault(GL_TEXTURE_2D);

    m_fbo = new globjects::Framebuffer();
    m_fbo->attachTexture(GL_COLOR_ATTACHMENT0, postprocessedFrame.data());

    m_screenAlignedQuad = new gloperate::ScreenAlignedQuad(
        globjects::Shader::fromFile(GL_FRAGMENT_SHADER, "data/shaders/postprocessing.frag")
    );
}

void PostprocessingStage::process()
{
    const auto screenSize = glm::vec2(viewport.data()->width(), viewport.data()->height());

    if (viewport.hasChanged())
    {
        resizeTexture(viewport.data()->width(), viewport.data()->height());
    }

    if (ssaoNoise.hasChanged())
    {
        generateNoiseTexture();
    }

    if (reflectionKernel.hasChanged())
    {
        generateReflectionKernelTexture();
    }

    generateKernelTexture();

    m_fbo->bind();
    m_fbo->setDrawBuffer(GL_COLOR_ATTACHMENT0);

    color.data()->bindActive(0);
    normal.data()->bindActive(1);
    depth.data()->bindActive(2);
    m_ssaoKernelTexture->bindActive(3);
    m_ssaoNoiseTexture->bindActive(4);
    worldPos.data()->bindActive(5);
    reflectMask.data()->bindActive(6);
    m_reflectionKernelTexture->bindActive(7);

    m_screenAlignedQuad->program()->setUniform("colorSampler", 0);
    m_screenAlignedQuad->program()->setUniform("normalSampler", 1);
    m_screenAlignedQuad->program()->setUniform("depthSampler", 2);
    m_screenAlignedQuad->program()->setUniform("ssaoKernelSampler", 3);
    m_screenAlignedQuad->program()->setUniform("ssaoNoiseSampler", 4);
    m_screenAlignedQuad->program()->setUniform("worldPosSampler", 5);
    m_screenAlignedQuad->program()->setUniform("reflectSampler", 6);
    m_screenAlignedQuad->program()->setUniform("reflectionKernelSampler", 7);

    m_screenAlignedQuad->program()->setUniform("useReflections", presetInformation.data().useReflections && useReflections.data());
    m_screenAlignedQuad->program()->setUniform("zThickness", presetInformation.data().zThickness);
    m_screenAlignedQuad->program()->setUniform("ssaoRadius", presetInformation.data().lightMaxShift);
    m_screenAlignedQuad->program()->setUniform("projectionMatrix", projection.data()->projection());
    m_screenAlignedQuad->program()->setUniform("projectionInverseMatrix", projection.data()->projectionInverted());
    m_screenAlignedQuad->program()->setUniform("normalMatrix", camera.data()->normal());
    m_screenAlignedQuad->program()->setUniform("view", camera.data()->view());
    m_screenAlignedQuad->program()->setUniform("farZ", projection.data()->zFar());
    m_screenAlignedQuad->program()->setUniform("screenSize", screenSize);
    m_screenAlignedQuad->program()->setUniform("samplerSizes", glm::vec4(ssaoKernelSize.data(), 1.f / ssaoKernelSize.data(), ssaoNoiseSize.data(), 1.f / ssaoNoiseSize.data()));
    m_screenAlignedQuad->program()->setUniform("cameraEye", camera.data()->eye());
    m_screenAlignedQuad->program()->setUniform("currentFrame", currentFrame.data() - 1);

    m_screenAlignedQuad->draw();

    m_fbo->unbind();
}

void PostprocessingStage::resizeTexture(int width, int height)
{
    postprocessedFrame.data()->image2D(0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    m_fbo->printStatus(true);
}

void PostprocessingStage::generateNoiseTexture()
{
    auto& noise = ssaoNoise.data();
    auto size = static_cast<int>(std::sqrt(noise.size()));

    auto texture = new globjects::Texture(gl::GL_TEXTURE_2D);
    texture->setParameter(gl::GL_TEXTURE_MIN_FILTER, gl::GL_NEAREST);
    texture->setParameter(gl::GL_TEXTURE_MAG_FILTER, gl::GL_NEAREST);
    texture->setParameter(gl::GL_TEXTURE_WRAP_S, gl::GL_MIRRORED_REPEAT);
    texture->setParameter(gl::GL_TEXTURE_WRAP_T, gl::GL_MIRRORED_REPEAT);

    texture->image2D(0, gl::GL_RGBA32F, glm::ivec2(size), 0, gl::GL_RGB, gl::GL_FLOAT, noise.data());

    m_ssaoNoiseTexture = texture;
}

void PostprocessingStage::generateKernelTexture()
{
    auto kernel = ssaoKernel.data();

    auto texture = new globjects::Texture(gl::GL_TEXTURE_1D);
    texture->setParameter(gl::GL_TEXTURE_MIN_FILTER, gl::GL_NEAREST);
    texture->setParameter(gl::GL_TEXTURE_MAG_FILTER, gl::GL_NEAREST);
    texture->setParameter(gl::GL_TEXTURE_WRAP_S, gl::GL_MIRRORED_REPEAT);

    texture->image1D(0, gl::GL_RGBA32F, kernel.size(), 0, gl::GL_RGB, gl::GL_FLOAT, kernel.data());

    m_ssaoKernelTexture = texture;
}

void PostprocessingStage::generateReflectionKernelTexture()
{
    auto kernel = reflectionKernel.data();

    auto texture = new globjects::Texture(gl::GL_TEXTURE_1D);
    texture->setParameter(gl::GL_TEXTURE_MIN_FILTER, gl::GL_NEAREST);
    texture->setParameter(gl::GL_TEXTURE_MAG_FILTER, gl::GL_NEAREST);
    texture->setParameter(gl::GL_TEXTURE_WRAP_S, gl::GL_MIRRORED_REPEAT);

    texture->image1D(0, gl::GL_RGBA32F, kernel.size(), 0, gl::GL_RGB, gl::GL_FLOAT, kernel.data());

    m_reflectionKernelTexture = texture;
}
