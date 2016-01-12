#pragma once

#include <chrono>

#include <gloperate/pipeline/PipelinePainter.h>

#include "mfs-painters-api.h"
#include "MultiFramePipeline.h"


namespace gloperate 
{
    class ResourceManager;
    class AbstractTargetFramebufferCapability;
    class AbstractViewportCapability;
    class AbstractPerspectiveProjectionCapability;
    class AbstractCameraCapability;
}


class MFS_PAINTERS_API MultiFramePainter : public gloperate::PipelinePainter
{
public:
    MultiFramePainter(gloperate::ResourceManager & resourceManager, const cpplocate::ModuleInfo & moduleInfo);
    virtual ~MultiFramePainter();

    int multiframeCount() const;
    float framesPerSecond() const;

protected:
    virtual void onInitialize() override;
    virtual void onPaint() override;


protected:
    MultiFramePipeline m_pipeline;

    float m_fps;
    std::chrono::time_point<std::chrono::steady_clock> m_lastTimepoint;

    /* Capabilities */
    gloperate::AbstractTargetFramebufferCapability * m_targetFramebufferCapability;
    gloperate::AbstractViewportCapability * m_viewportCapability;
    gloperate::AbstractPerspectiveProjectionCapability * m_projectionCapability;
    gloperate::AbstractCameraCapability * m_cameraCapability;
};
