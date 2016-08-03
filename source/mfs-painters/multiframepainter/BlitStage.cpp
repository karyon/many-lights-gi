#include "BlitStage.h"

#include <glbinding/gl/enum.h>
#include <glbinding/gl/boolean.h>
#include <glbinding/gl/bitfield.h>
#include <glbinding/gl/functions.h>

#include <globjects/Texture.h>
#include <globjects/Framebuffer.h>

#include <gloperate/painter/AbstractViewportCapability.h>
#include <gloperate/primitives/ScreenAlignedQuad.h>

#include "MultiFramePainter.h"
#include "PerfCounter.h"


using namespace gl;


BlitStage::BlitStage()
: m_currentBuffer("Accumulation Buffer")
{
}

void BlitStage::initialize()
{
    m_fbo = new globjects::Framebuffer();
    m_fbo->attachTexture(GL_DEPTH_ATTACHMENT, depth);

    m_screenAlignedQuad = new gloperate::ScreenAlignedQuad(
        globjects::Shader::fromFile(GL_FRAGMENT_SHADER, "data/shaders/blit.frag")
    );

    m_currentMipLevel = 0;
}


void BlitStage::initProperties(MultiFramePainter& painter)
{
    std::vector<std::string> bufferNames;
    for (auto buffer : m_buffers)
        bufferNames.push_back(buffer->name());
    painter.addProperty<std::string>("Buffer",
        [this]() { return m_currentBuffer; },
        [this](const std::string & value) {
            m_currentBuffer = value;
        })->setOption("choices", bufferNames);
    painter.addProperty<int>("MipLevel",
        [this]() { return m_currentMipLevel; },
        [this](const int & value) {
            m_currentMipLevel = value;
    });
}

void BlitStage::process()
{
    AutoGLPerfCounter c("Blit");

    globjects::ref_ptr<globjects::Texture> buffer;
    for (auto b : m_buffers) {
        std::string name = b->name();
        if (b->name() == m_currentBuffer) {
            buffer = b;
            break;
        }
    }

    bool singleChannel =
        m_currentBuffer.find("Occlusion") != std::string::npos ||
        m_currentBuffer.find("Pull") != std::string::npos ||
        m_currentBuffer.find("Push") != std::string::npos ||
        m_currentBuffer.find("softrender") != std::string::npos ||
        m_currentBuffer.find("Depth") != std::string::npos;
    m_screenAlignedQuad->program()->setUniform("singleChannel", singleChannel);

    buffer->bindActive(0);
    buffer->bindActive(1);
    m_screenAlignedQuad->program()->setUniform("someBuffer", 0);
    m_screenAlignedQuad->program()->setUniform("softRenderBuffer", 1);
    m_screenAlignedQuad->program()->setUniform("mipLevel", m_currentMipLevel);

    bool softRenderBufferActive = m_currentBuffer.find("softrender") != std::string::npos;
    m_screenAlignedQuad->program()->setUniform("softRenderBufferActive", softRenderBufferActive);

    auto viewportRect = std::array<GLint, 4>{ {
        viewport->x(),
        viewport->y(),
        viewport->width(),
        viewport->height()
    }};
    auto virtualViewportRect = std::array<GLint, 4>{ {
        virtualViewport->x(),
        virtualViewport->y(),
        virtualViewport->width(),
        virtualViewport->height()
    }};

    glViewport(
        viewport->x(),
        viewport->y(),
        viewport->width(),
        viewport->height()
    );

    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);

    m_screenAlignedQuad->draw();

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    auto defaultFbo = globjects::Framebuffer::defaultFBO();
    m_fbo->blit(GL_COLOR_ATTACHMENT0, virtualViewportRect, defaultFbo, GL_BACK_LEFT, viewportRect, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}
