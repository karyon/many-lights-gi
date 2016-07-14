#pragma once

#include <vector>
#include <string>

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

class MultiFramePainter;


class BlitStage
{
public:
    BlitStage();

    void initialize();
	void initProperties(MultiFramePainter& painter);

    void process();

    gloperate::AbstractViewportCapability * viewport;

    globjects::ref_ptr<globjects::Texture> accumulation;
    globjects::ref_ptr<globjects::Texture> depth;

	std::vector<globjects::ref_ptr<globjects::Texture>> m_buffers;

protected:
    globjects::ref_ptr<globjects::Framebuffer> m_fbo;
    std::string m_currentBuffer;
    int m_currentMipLevel;
    globjects::ref_ptr<gloperate::ScreenAlignedQuad> m_screenAlignedQuad;
};
