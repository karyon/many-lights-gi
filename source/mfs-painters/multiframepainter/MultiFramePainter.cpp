
#include "MultiFramePainter.h"

#include <random>
#include <memory>
#include <iostream>

#include <cpplocate/ModuleInfo.h>

#include <iozeug/FilePath.h>

#include <glbinding/gl/functions.h>
#include <glbinding/gl/enum.h>
#include <glbinding/gl/boolean.h>

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
#include "BlitStage.h"
#include "PerfCounter.h"
#include "ImperfectShadowmap.h"
#include "ClusteredShading.h"
#include "VPLProcessor.h"


using namespace reflectionzeug;
using namespace globjects;


MultiFramePainter::MultiFramePainter(gloperate::ResourceManager & resourceManager, const cpplocate::ModuleInfo & moduleInfo)
: Painter("MultiFramePainter", resourceManager, moduleInfo)
, resourceManager(resourceManager)
, preset(Preset::CrytekSponza)
, m_useFullHD(false)
{
    // Setup painter
    m_targetFramebufferCapability = addCapability(new gloperate::TargetFramebufferCapability());
    m_virtualViewportCapability = new gloperate::ViewportCapability();
    m_viewportCapability = addCapability(new gloperate::ViewportCapability());
    m_projectionCapability = addCapability(new gloperate::PerspectiveProjectionCapability(m_virtualViewportCapability));
    m_cameraCapability = addCapability(new gloperate::CameraCapability());


    modelLoadingStage = std::make_unique<ModelLoadingStage>();
    kernelGenerationStage = std::make_unique<KernelGenerationStage>();
    rasterizationStage = std::make_unique<RasterizationStage>("GBuffer", *modelLoadingStage, *kernelGenerationStage, false);
    giStage = std::make_unique<GIStage>(*modelLoadingStage, *kernelGenerationStage);
    ssaoStage = std::make_unique<SSAOStage>(*kernelGenerationStage, *modelLoadingStage);
    deferredShadingStage = std::make_unique<DeferredShadingStage>();
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

}

MultiFramePainter::~MultiFramePainter()
{
}

void MultiFramePainter::onInitialize()
{
    this->addProperty<bool>("useFullHD",
        [this]() { return m_useFullHD; },
        [this](const bool & value) {
            m_useFullHD = value;
            if (m_useFullHD)
                m_virtualViewportCapability->setViewport(0, 0, 1920, 1080);
            else
                m_viewportCapability->setChanged(true); // actual update happens below

    });

    gloperate::registerNamedStrings("data/shaders", "glsl", true);

    // disable debug group console output
    gl::glDebugMessageControl(gl::GL_DEBUG_SOURCE_APPLICATION, gl::GL_DEBUG_TYPE_PUSH_GROUP, gl::GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, gl::GL_FALSE);
    gl::glDebugMessageControl(gl::GL_DEBUG_SOURCE_APPLICATION, gl::GL_DEBUG_TYPE_POP_GROUP, gl::GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, gl::GL_FALSE);

    kernelGenerationStage->initialize();

    modelLoadingStage->loadScene(preset);

    rasterizationStage->projection = m_projectionCapability;
    rasterizationStage->camera = m_cameraCapability;
    rasterizationStage->viewport = m_virtualViewportCapability;
    rasterizationStage->useDOF = useDOF;
    rasterizationStage->initialize();
    rasterizationStage->initProperties(*this);
    rasterizationStage->loadPreset(modelLoadingStage->getCurrentPresetInformation());

    giStage->viewport = m_virtualViewportCapability;
    giStage->camera = m_cameraCapability;
    giStage->projection = m_projectionCapability;
    giStage->faceNormalBuffer = rasterizationStage->faceNormalBuffer;
    giStage->depthBuffer = rasterizationStage->depthBuffer;
    giStage->initialize();
    giStage->initProperties(*this);

    ssaoStage->viewport = m_virtualViewportCapability;
    ssaoStage->camera = m_cameraCapability;
    ssaoStage->projection = m_projectionCapability;
    ssaoStage->faceNormalBuffer = rasterizationStage->faceNormalBuffer;
    ssaoStage->normalBuffer = rasterizationStage->normalBuffer;
    ssaoStage->depthBuffer = rasterizationStage->depthBuffer;
    ssaoStage->initialize();

    deferredShadingStage->viewport = m_virtualViewportCapability;
    deferredShadingStage->camera = m_cameraCapability;
    deferredShadingStage->projection = m_projectionCapability;
    deferredShadingStage->diffuseBuffer = rasterizationStage->diffuseBuffer;
    deferredShadingStage->specularBuffer = rasterizationStage->specularBuffer;
    deferredShadingStage->giBuffer = giStage->giBlurFinalBuffer;
    deferredShadingStage->occlusionBuffer = ssaoStage->occlusionBuffer;
    deferredShadingStage->faceNormalBuffer = rasterizationStage->faceNormalBuffer;
    deferredShadingStage->normalBuffer = rasterizationStage->normalBuffer;
    deferredShadingStage->depthBuffer = rasterizationStage->depthBuffer;
    deferredShadingStage->shadowmap = giStage->rsmRenderer->vsmBuffer;
    deferredShadingStage->biasedShadowTransform = &giStage->vplProcessor->biasedShadowTransform;
    deferredShadingStage->lightDirection = &giStage->lightDirection;
    deferredShadingStage->lightPosition = &giStage->lightPosition;
    deferredShadingStage->lightIntensity = &giStage->lightIntensity;
    deferredShadingStage->initialize();
    deferredShadingStage->initProperties(*this);

    blitStage->viewport = m_viewportCapability;
    blitStage->virtualViewport = m_virtualViewportCapability;
    blitStage->depth = rasterizationStage->depthBuffer;

    blitStage->m_buffers = {
        rasterizationStage->diffuseBuffer,
        rasterizationStage->specularBuffer,
        rasterizationStage->normalBuffer,
        rasterizationStage->faceNormalBuffer,
        rasterizationStage->depthBuffer,
        giStage->rsmRenderer->diffuseBuffer,
        giStage->rsmRenderer->specularBuffer,
        giStage->rsmRenderer->normalBuffer,
        giStage->rsmRenderer->faceNormalBuffer,
        giStage->rsmRenderer->vsmBuffer,
        giStage->rsmRenderer->depthBuffer,
        giStage->ism->depthBuffer,
        giStage->ism->softrenderBuffer,
        giStage->ism->pullBuffer,
        giStage->ism->pushBuffer,
        giStage->ism->pushPullResultBuffer,
        giStage->clusteredShading->lightLists,
        giStage->giBuffer,
        giStage->giBlurTempBuffer,
        giStage->giBlurFinalBuffer,
        ssaoStage->occlusionBuffer,
        deferredShadingStage->shadedFrame,
    };

    blitStage->initialize();
    blitStage->initProperties(*this);
}

void MultiFramePainter::onPaint()
{
    modelLoadingStage->tick();

    if (!m_useFullHD && m_viewportCapability->hasChanged()) {
        m_virtualViewportCapability->setViewport(0, 0, m_viewportCapability->width(), m_viewportCapability->height());
    }

    {
    AutoGLPerfCounter c("GBuffer");
    rasterizationStage->process();
    }
    giStage->process();
    ssaoStage->process();
    deferredShadingStage->process();
    blitStage->process();

    m_virtualViewportCapability->setChanged(false);
    m_viewportCapability->setChanged(false);
    m_cameraCapability->setChanged(false);
    m_projectionCapability->setChanged(false);
}

std::string MultiFramePainter::getPerfCounterString() const
{
    return PerfCounter::generateString();
}