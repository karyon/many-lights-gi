
#include "MultiFramePainter.h"

#include <random>
#include <memory>
#include <iostream>

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
#include "GIStage.h"
#include "DeferredShadingStage.h"
#include "SSAOStage.h"
#include "FrameAccumulationStage.h"
#include "BlitStage.h"
#include "PerfCounter.h"
#include "Shadowmap.h"


using namespace reflectionzeug;
using namespace globjects;
using namespace gloperate;


MultiFramePainter::MultiFramePainter(ResourceManager & resourceManager, const cpplocate::ModuleInfo & moduleInfo)
: Painter("MultiFramePainter", resourceManager, moduleInfo)
, resourceManager(resourceManager)
, preset(Preset::CrytekSponza)
{
    // Setup painter
    m_targetFramebufferCapability = addCapability(new gloperate::TargetFramebufferCapability());
    m_viewportCapability = addCapability(new gloperate::ViewportCapability());
    m_projectionCapability = addCapability(new gloperate::PerspectiveProjectionCapability(m_viewportCapability));
    m_cameraCapability = addCapability(new gloperate::CameraCapability());


    modelLoadingStage = std::make_unique<ModelLoadingStage>(preset);
    kernelGenerationStage = std::make_unique<KernelGenerationStage>();
    rasterizationStage = std::make_unique<RasterizationStage>(*modelLoadingStage, *kernelGenerationStage);
    giStage = std::make_unique<GIStage>(*modelLoadingStage, *kernelGenerationStage);
    ssaoStage = std::make_unique<SSAOStage>(*kernelGenerationStage, modelLoadingStage->getCurrentPreset());
    deferredShadingStage = std::make_unique<DeferredShadingStage>();
    frameAccumulationStage = std::make_unique<FrameAccumulationStage>();
    blitStage = std::make_unique<BlitStage>();

    modelLoadingStage->resourceManager = &resourceManager;

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
    giStage->initProperties(*this);
    deferredShadingStage->initProperties(*this);
}

MultiFramePainter::~MultiFramePainter()
{
}

void MultiFramePainter::onInitialize()
{
    gloperate::registerNamedStrings("data/shaders", "glsl", true);

    kernelGenerationStage->initialize();

    rasterizationStage->projection = m_projectionCapability;
    rasterizationStage->camera = m_cameraCapability;
    rasterizationStage->viewport = m_viewportCapability;
    rasterizationStage->useDOF = useDOF;
    rasterizationStage->initialize();
    rasterizationStage->loadPreset(modelLoadingStage->getCurrentPreset());

    giStage->viewport = m_viewportCapability;
    giStage->camera = m_cameraCapability;
    giStage->projection = m_projectionCapability;
    giStage->faceNormalBuffer = rasterizationStage->faceNormalBuffer;
    giStage->depthBuffer = rasterizationStage->depthBuffer;
    giStage->initialize();

    ssaoStage->viewport = m_viewportCapability;
    ssaoStage->camera = m_cameraCapability;
    ssaoStage->projection = m_projectionCapability;
    ssaoStage->faceNormalBuffer = rasterizationStage->faceNormalBuffer;
    ssaoStage->normalBuffer = rasterizationStage->normalBuffer;
    ssaoStage->depthBuffer = rasterizationStage->depthBuffer;
    ssaoStage->initialize();

    deferredShadingStage->viewport = m_viewportCapability;
    deferredShadingStage->camera = m_cameraCapability;
    deferredShadingStage->projection = m_projectionCapability;
    deferredShadingStage->diffuseBuffer = rasterizationStage->diffuseBuffer;
    deferredShadingStage->specularBuffer = rasterizationStage->specularBuffer;
    deferredShadingStage->giBuffer = giStage->giBuffer;
    deferredShadingStage->occlusionBuffer = ssaoStage->occlusionBuffer;
    deferredShadingStage->faceNormalBuffer = rasterizationStage->faceNormalBuffer;
    deferredShadingStage->normalBuffer = rasterizationStage->normalBuffer;
    deferredShadingStage->depthBuffer = rasterizationStage->depthBuffer;
    deferredShadingStage->shadowmap = giStage->shadowmap->distanceTexture();
    deferredShadingStage->biasedShadowTransform = &giStage->biasedShadowTransform;
    deferredShadingStage->lightDirection = &giStage->lightDirection;
    deferredShadingStage->lightPosition = &giStage->lightPosition;
    deferredShadingStage->initialize();

    frameAccumulationStage->viewport = m_viewportCapability;
    frameAccumulationStage->currentFrame = rasterizationStage->currentFrame;
    frameAccumulationStage->frame = deferredShadingStage->shadedFrame;
    frameAccumulationStage->depth = rasterizationStage->depthBuffer;
    frameAccumulationStage->initialize();

    blitStage->viewport = m_viewportCapability;
    blitStage->accumulation = frameAccumulationStage->accumulation;
    blitStage->depth = frameAccumulationStage->depth;
    blitStage->initialize();
}

void MultiFramePainter::onPaint()
{
    {
    AutoGLPerfCounter c("GBuffer");
    rasterizationStage->process();
    }
    giStage->process();
    ssaoStage->process();
    deferredShadingStage->process();
    frameAccumulationStage->process();
    blitStage->process();

    m_viewportCapability->setChanged(false);
    m_cameraCapability->setChanged(false);
    m_projectionCapability->setChanged(false);
}

std::string MultiFramePainter::getPerfCounterString() const
{
    return PerfCounter::generateString();
}