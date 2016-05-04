#pragma once

#include <chrono>
#include <memory>

#include <gloperate/painter/Painter.h>

#include "mfs-painters-api.h"

#include "Preset.h"


namespace gloperate 
{
    class ResourceManager;
    class AbstractTargetFramebufferCapability;
    class AbstractViewportCapability;
    class AbstractPerspectiveProjectionCapability;
    class AbstractCameraCapability;
    class ResourceManager;
}

class ModelLoadingStage;
class KernelGenerationStage;
class RasterizationStage;
class GIStage;
class DeferredShadingStage;
class SSAOStage;
class FrameAccumulationStage;
class BlitStage;


class MFS_PAINTERS_API MultiFramePainter : public gloperate::Painter
{
public:
    MultiFramePainter(gloperate::ResourceManager & resourceManager, const cpplocate::ModuleInfo & moduleInfo);
    virtual ~MultiFramePainter();

    // Viewer.cpp can't access PerfCounter::generateString(), probably because different compilation unit
    std::string getPerfCounterString() const;

    gloperate::ResourceManager& resourceManager;
    Preset preset;
    bool useDOF;

protected:
    virtual void onInitialize() override;
    virtual void onPaint() override;


protected:

    /* Capabilities */
    gloperate::AbstractTargetFramebufferCapability * m_targetFramebufferCapability;
    gloperate::AbstractViewportCapability * m_viewportCapability;
    gloperate::AbstractPerspectiveProjectionCapability * m_projectionCapability;
    gloperate::AbstractCameraCapability * m_cameraCapability;

    std::unique_ptr<ModelLoadingStage> modelLoadingStage;
    std::unique_ptr<KernelGenerationStage> kernelGenerationStage;
    std::unique_ptr<RasterizationStage> rasterizationStage;
    std::unique_ptr<GIStage> giStage;
    std::unique_ptr<SSAOStage> ssaoStage;
    std::unique_ptr<DeferredShadingStage> deferredShadingStage;
    std::unique_ptr<FrameAccumulationStage> frameAccumulationStage;
    std::unique_ptr<BlitStage> blitStage;
};
