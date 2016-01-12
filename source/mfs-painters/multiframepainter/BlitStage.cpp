#include "BlitStage.h"

#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>

#include <globjects/Texture.h>
#include <globjects/Framebuffer.h>

#include <gloperate/painter/AbstractViewportCapability.h>

using namespace gl;

BlitStage::BlitStage()
{
    addInput("accumulation", accumulation);
    addInput("depth", depth);
    addInput("viewport", viewport);
}

void BlitStage::initialize()
{
    alwaysProcess(true);
    m_fbo = new globjects::Framebuffer();
    m_fbo->attachTexture(GL_COLOR_ATTACHMENT0, accumulation.data());
    m_fbo->attachTexture(GL_DEPTH_ATTACHMENT, depth.data());
}

void BlitStage::process()
{
    auto rect = std::array<GLint, 4>{{
        viewport.data()->x(),
        viewport.data()->y(),
        viewport.data()->width(),
        viewport.data()->height()
    }};

    auto defaultFbo = globjects::Framebuffer::defaultFBO();
    m_fbo->blit(GL_COLOR_ATTACHMENT0, rect, defaultFbo, GL_BACK_LEFT, rect, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}
