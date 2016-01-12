#pragma once

#include <glkernel/Kernel.h>

#include <globjects/base/ref_ptr.h>

#include <gloperate/pipeline/AbstractStage.h>
#include <gloperate/pipeline/InputSlot.h>

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

class KernelGenerationStage : public gloperate::AbstractStage
{
public:
    KernelGenerationStage();

    virtual void initialize() override;
    virtual void process() override;

    gloperate::InputSlot<int> multiFrameCount;

    gloperate::Data<glkernel::kernel2> antiAliasingKernel;
    gloperate::Data<glkernel::kernel2> depthOfFieldKernel;
    gloperate::Data<glkernel::kernel2> shadowKernel;
    gloperate::Data<glkernel::kernel3> ssaoKernel;
    gloperate::Data<glkernel::kernel3> reflectionKernel;

    gloperate::Data<std::vector<glm::vec3>> ssaoNoise;
    gloperate::Data<int> ssaoKernelSize;
    gloperate::Data<int> ssaoNoiseSize;

    gloperate::Data<globjects::ref_ptr<globjects::Texture>> postprocessedFrame;

};
