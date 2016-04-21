#include "BlitStage.h"

#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>

#include <globjects/Texture.h>
#include <globjects/Framebuffer.h>

#include <gloperate/painter/AbstractViewportCapability.h>

using namespace gl;

BlitStage::BlitStage()
{
}

void BlitStage::initialize()
{
    m_fbo = new globjects::Framebuffer();
    m_fbo->attachTexture(GL_COLOR_ATTACHMENT0, accumulation);
    m_fbo->attachTexture(GL_DEPTH_ATTACHMENT, depth);
}

void BlitStage::process()
{
    auto rect = std::array<GLint, 4>{{
        viewport->x(),
        viewport->y(),
        viewport->width(),
        viewport->height()
    }};

    auto defaultFbo = globjects::Framebuffer::defaultFBO();
    m_fbo->blit(GL_COLOR_ATTACHMENT0, rect, defaultFbo, GL_BACK_LEFT, rect, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}
