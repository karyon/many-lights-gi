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
    class ScreenAlignedQuad;
}

class FrameAccumulationStage
{
public:
    FrameAccumulationStage();

    void initialize();
    void process();

    gloperate::AbstractViewportCapability * viewport;
    int currentFrame;

    globjects::ref_ptr<globjects::Texture> frame;
    globjects::ref_ptr<globjects::Texture> depth;

    globjects::ref_ptr<globjects::Texture> accumulation;


protected:

    void resizeTexture(int width, int height);

    globjects::ref_ptr<globjects::Framebuffer> m_fbo;
    globjects::ref_ptr<gloperate::ScreenAlignedQuad> m_screenAlignedQuad;
};
