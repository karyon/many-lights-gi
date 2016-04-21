#pragma once

#include <globjects/base/ref_ptr.h>


namespace globjects
{
    class Framebuffer;
    class Texture;
}

namespace gloperate
{
    class AbstractViewportCapability;
}

class BlitStage
{
public:
    BlitStage();

    void initialize();
    void process();

    gloperate::AbstractViewportCapability * viewport;

    globjects::ref_ptr<globjects::Texture> accumulation;
    globjects::ref_ptr<globjects::Texture> depth;


protected:

    globjects::ref_ptr<globjects::Framebuffer> m_fbo;
};
