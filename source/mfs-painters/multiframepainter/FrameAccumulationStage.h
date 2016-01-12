#pragma once

#include <globjects/base/ref_ptr.h>

#include <gloperate/pipeline/AbstractStage.h>
#include <gloperate/pipeline/InputSlot.h>


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

class FrameAccumulationStage : public gloperate::AbstractStage
{
public:
    FrameAccumulationStage();

    virtual void initialize() override;

    gloperate::InputSlot<gloperate::AbstractViewportCapability *> viewport;
    gloperate::InputSlot<int> currentFrame;

    gloperate::InputSlot<globjects::ref_ptr<globjects::Texture>> frame;
    gloperate::InputSlot<globjects::ref_ptr<globjects::Texture>> depth;

    gloperate::Data<globjects::ref_ptr<globjects::Texture>> accumulation;


protected:
    virtual void process() override;

    void resizeTexture(int width, int height);

    globjects::ref_ptr<globjects::Framebuffer> m_fbo;
    globjects::ref_ptr<gloperate::ScreenAlignedQuad> m_screenAlignedQuad;
};
