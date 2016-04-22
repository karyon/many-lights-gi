
#include "MultiFramePainter.h"

#include <random>

#include <cpplocate/ModuleInfo.h>

#include <iozeug/FilePath.h>

#include <gloperate/resources/ResourceManager.h>
#include <gloperate/base/registernamedstrings.h>
#include <gloperate/painter/ViewportCapability.h>
#include <gloperate/painter/TargetFramebufferCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/CameraCapability.h>


#include "ModelLoadingStage.h"
#include "KernelGenerationStage.h"
#include "RasterizationStage.h"
#include "PostprocessingStage.h"
#include "FrameAccumulationStage.h"
#include "BlitStage.h"


using namespace reflectionzeug;
using namespace globjects;
using namespace gloperate;


MultiFramePainter::MultiFramePainter(ResourceManager & resourceManager, const cpplocate::ModuleInfo & moduleInfo)
: Painter("MultiFramePainter", resourceManager, moduleInfo)
, m_multiFrameCount(64)
, resourceManager(resourceManager)
, multiFrameCount(64)
, preset(Preset::CrytekSponza)
{


    // Setup painter
    m_targetFramebufferCapability = addCapability(new gloperate::TargetFramebufferCapability());
    m_viewportCapability = addCapability(new gloperate::ViewportCapability());
    m_projectionCapability = addCapability(new gloperate::PerspectiveProjectionCapability(m_viewportCapability));
    m_cameraCapability = addCapability(new gloperate::CameraCapability());


    modelLoadingStage = std::make_unique<ModelLoadingStage>();
    kernelGenerationStage = std::make_unique<KernelGenerationStage>();
    postprocessingStage = std::make_unique<PostprocessingStage>(*kernelGenerationStage);
    frameAccumulationStage = std::make_unique<FrameAccumulationStage>();
    blitStage = std::make_unique<BlitStage>();
    rasterizationStage = std::make_unique<RasterizationStage>(*modelLoadingStage, *kernelGenerationStage, *postprocessingStage, *frameAccumulationStage, *blitStage);

    modelLoadingStage->resourceManager = &resourceManager;
    modelLoadingStage->preset = preset;

    rasterizationStage->projection = m_projectionCapability;
    rasterizationStage->camera = m_cameraCapability;
    rasterizationStage->viewport = m_viewportCapability;
    rasterizationStage->multiFrameCount = multiFrameCount;
    rasterizationStage->useDOF = useDOF;

    postprocessingStage->viewport = m_viewportCapability;
    postprocessingStage->camera = m_cameraCapability;
    postprocessingStage->projection = m_projectionCapability;
    postprocessingStage->color = rasterizationStage->color;
    postprocessingStage->normal = rasterizationStage->normal;
    postprocessingStage->depth = rasterizationStage->depth;
    postprocessingStage->worldPos = rasterizationStage->worldPos;

    frameAccumulationStage->viewport = m_viewportCapability;
    frameAccumulationStage->currentFrame = rasterizationStage->currentFrame;
    frameAccumulationStage->frame = postprocessingStage->postprocessedFrame;
    frameAccumulationStage->depth = rasterizationStage->depth;

    blitStage->viewport = m_viewportCapability;
    blitStage->accumulation = frameAccumulationStage->accumulation;
    blitStage->depth = rasterizationStage->depth;


    // Get data path
    std::string dataPath = moduleInfo.value("dataPath");
    dataPath = iozeug::FilePath(dataPath).path();
    if (dataPath.size() > 0)
    {
        dataPath = dataPath + "/";
    }
    else
    {
        dataPath = "data/";
    }

    rasterizationStage->initProperties(*this);
}

MultiFramePainter::~MultiFramePainter()
{
}

int MultiFramePainter::multiframeCount() const
{
    return m_multiFrameCount;
}

float MultiFramePainter::framesPerSecond() const
{
    return m_fps;
}

void MultiFramePainter::onInitialize()
{
    gloperate::registerNamedStrings("data/shaders", "glsl", true);
    rasterizationStage->initialize();
}

void MultiFramePainter::onPaint()
{
    using namespace std::chrono;

    auto now = steady_clock::now();
    auto duration = duration_cast<milliseconds>(now - m_lastTimepoint);
    m_fps = 1000.0f / duration.count();
    m_lastTimepoint = now;
    rasterizationStage->process();
}
