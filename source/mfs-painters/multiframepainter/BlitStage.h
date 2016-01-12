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
}

class BlitStage : public gloperate::AbstractStage
{
public:
    BlitStage();

    virtual void initialize() override;

    gloperate::InputSlot<gloperate::AbstractViewportCapability *> viewport;

    gloperate::InputSlot<globjects::ref_ptr<globjects::Texture>> accumulation;
    gloperate::InputSlot<globjects::ref_ptr<globjects::Texture>> depth;


protected:
    virtual void process() override;

    globjects::ref_ptr<globjects::Framebuffer> m_fbo;
};
