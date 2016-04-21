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
class PostprocessingStage;
class FrameAccumulationStage;
class BlitStage;


class MFS_PAINTERS_API MultiFramePainter : public gloperate::Painter
{
public:
    MultiFramePainter(gloperate::ResourceManager & resourceManager, const cpplocate::ModuleInfo & moduleInfo);
    virtual ~MultiFramePainter();

    int multiframeCount() const;
    float framesPerSecond() const;


    gloperate::ResourceManager& resourceManager;
    int multiFrameCount;
    Preset preset;
    bool useDOF;

protected:
    virtual void onInitialize() override;
    virtual void onPaint() override;


protected:

    float m_fps;
    std::chrono::time_point<std::chrono::steady_clock> m_lastTimepoint;

    /* Capabilities */
    gloperate::AbstractTargetFramebufferCapability * m_targetFramebufferCapability;
    gloperate::AbstractViewportCapability * m_viewportCapability;
    gloperate::AbstractPerspectiveProjectionCapability * m_projectionCapability;
    gloperate::AbstractCameraCapability * m_cameraCapability;

    int m_multiFrameCount;

    std::unique_ptr<ModelLoadingStage> modelLoadingStage;
    std::unique_ptr<KernelGenerationStage> kernelGenerationStage;
    std::unique_ptr<RasterizationStage> rasterizationStage;
    std::unique_ptr<PostprocessingStage> postprocessingStage;
    std::unique_ptr<FrameAccumulationStage> frameAccumulationStage;
    std::unique_ptr<BlitStage> blitStage;
};
