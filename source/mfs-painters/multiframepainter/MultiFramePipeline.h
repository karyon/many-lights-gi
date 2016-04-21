#pragma once

#include <gloperate/pipeline/AbstractPipeline.h>
#include <gloperate/resources/ResourceManager.h>

#include "Preset.h"

namespace gloperate
{
    class AbstractPerspectiveProjectionCapability;
    class AbstractCameraCapability;
    class AbstractViewportCapability;
}

class MultiFramePipeline : public gloperate::AbstractPipeline
{
public:
    MultiFramePipeline(gloperate::ResourceManager& resourceManager);

    gloperate::Data<gloperate::ResourceManager *> resourceManager;
    gloperate::Data<int> multiFrameCount;
    gloperate::Data<Preset> preset;
    gloperate::Data<bool> useReflections;
    gloperate::Data<bool> useDOF;

    gloperate::Data<gloperate::AbstractViewportCapability *> viewport;
    gloperate::Data<gloperate::AbstractCameraCapability *> camera;
    gloperate::Data<gloperate::AbstractPerspectiveProjectionCapability *> projection;
};
