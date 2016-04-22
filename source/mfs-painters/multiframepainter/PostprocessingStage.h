#pragma once

#include <glkernel/Kernel.h>

#include <globjects/base/ref_ptr.h>

#include "TypeDefinitions.h"

namespace globjects
{
    class Framebuffer;
    class Texture;
}

namespace gloperate
{
    class AbstractViewportCapability;
    class AbstractPerspectiveProjectionCapability;
    class AbstractCameraCapability;

    class ScreenAlignedQuad;
}

class KernelGenerationStage;

class PostprocessingStage
{
public:
    PostprocessingStage(KernelGenerationStage& kernelGenerationStage);

    void initialize();
    void process();

    gloperate::AbstractPerspectiveProjectionCapability * projection;
    gloperate::AbstractViewportCapability * viewport;
    gloperate::AbstractCameraCapability * camera;
    PresetInformation presetInformation;
    int ssaoKernelSize;
    int ssaoNoiseSize;

    globjects::ref_ptr<globjects::Texture> color;
    globjects::ref_ptr<globjects::Texture> normal;
    globjects::ref_ptr<globjects::Texture> depth;
    globjects::ref_ptr<globjects::Texture> worldPos;

    globjects::ref_ptr<globjects::Texture> postprocessedFrame;

protected:

    void resizeTexture(int width, int height);
    void generateNoiseTexture();
    void createKernelTexture();
    void updateKernelTexture();

    globjects::ref_ptr<globjects::Framebuffer> m_fbo;
    globjects::ref_ptr<gloperate::ScreenAlignedQuad> m_screenAlignedQuad;

    globjects::ref_ptr<globjects::Texture> m_ssaoKernelTexture;
    globjects::ref_ptr<globjects::Texture> m_ssaoNoiseTexture;
    KernelGenerationStage& m_kernelGenerationStage;
};
