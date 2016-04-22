#include "FrameAccumulationStage.h"

#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>
#include <glbinding/gl/boolean.h>

#include <glm/vec4.hpp>

#include <globjects/Texture.h>
#include <globjects/Program.h>
#include <globjects/Framebuffer.h>

#include <gloperate/primitives/ScreenAlignedQuad.h>
#include <gloperate/painter/AbstractViewportCapability.h>

using namespace gl;

FrameAccumulationStage::FrameAccumulationStage()
{
}

void FrameAccumulationStage::initialize()
{
    accumulation = globjects::Texture::createDefault(GL_TEXTURE_2D);

    m_fbo = new globjects::Framebuffer();
    m_fbo->attachTexture(GL_COLOR_ATTACHMENT0, accumulation);
    m_fbo->attachTexture(GL_DEPTH_ATTACHMENT, depth);

    m_screenAlignedQuad = new gloperate::ScreenAlignedQuad(
        globjects::Shader::fromFile(GL_FRAGMENT_SHADER, "data/shaders/accumulation.frag")
    );
}

void FrameAccumulationStage::process()
{
    if (viewport->hasChanged())
        resizeTexture(viewport->width(), viewport->height());

    m_fbo->clearBuffer(GL_COLOR, 0, glm::vec4(0.0f));

    glDepthMask(GL_FALSE);
    m_fbo->bind();
    m_fbo->setDrawBuffer(GL_COLOR_ATTACHMENT0);

    accumulation->bindActive(0);
    frame->bindActive(1);
    m_screenAlignedQuad->program()->setUniform("accumBuffer", 0);
    m_screenAlignedQuad->program()->setUniform("frameBuffer", 1);
    m_screenAlignedQuad->program()->setUniform("weight", 1.0f / currentFrame);

    m_screenAlignedQuad->draw();
    accumulation->unbindActive(0);
    frame->unbindActive(1);

    m_fbo->unbind();
    glDepthMask(GL_TRUE);
}

void FrameAccumulationStage::resizeTexture(int width, int height)
{
    accumulation->image2D(0, GL_RGBA32F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
    m_fbo->printStatus(true);
}
