
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


using namespace reflectionzeug;
using namespace globjects;
using namespace gloperate;


MultiFramePainter::MultiFramePainter(ResourceManager & resourceManager, const cpplocate::ModuleInfo & moduleInfo)
: PipelinePainter("MultiFramePainter", resourceManager, moduleInfo, m_pipeline)
, m_pipeline(resourceManager)
{
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

    m_pipeline.resourceManager.setData(&resourceManager);

    // Setup painter
    m_targetFramebufferCapability = addCapability(new gloperate::TargetFramebufferCapability());
    m_viewportCapability = addCapability(new gloperate::ViewportCapability());
    m_projectionCapability = addCapability(new gloperate::PerspectiveProjectionCapability(m_viewportCapability));
    m_cameraCapability = addCapability(new gloperate::CameraCapability());

    m_pipeline.viewport.setData(m_viewportCapability);
    m_pipeline.projection.setData(m_projectionCapability);
    m_pipeline.camera.setData(m_cameraCapability);

    m_cameraCapability->changed.connect([this](){ m_pipeline.camera.invalidate(); });
    m_viewportCapability->changed.connect([this]() { m_pipeline.viewport.invalidate(); });
    m_projectionCapability->changed.connect([this]() { m_pipeline.projection.invalidate(); });

    addProperty(createProperty("Preset", m_pipeline.preset));
    addProperty(createProperty("MultiframeCount", m_pipeline.multiFrameCount));
    addProperty(createProperty("Reflections", m_pipeline.useReflections));
    addProperty(createProperty("DepthOfField", m_pipeline.useDOF));
}

MultiFramePainter::~MultiFramePainter()
{
}

int MultiFramePainter::multiframeCount() const
{
    return m_pipeline.getOutput<int>("currentFrame")->data();
}

float MultiFramePainter::framesPerSecond() const
{
    return m_fps;
}

void MultiFramePainter::onInitialize()
{
    gloperate::registerNamedStrings("data/shaders", "glsl", true);

    PipelinePainter::onInitialize();
}

void MultiFramePainter::onPaint()
{
    using namespace std::chrono;

    auto now = steady_clock::now();
    auto duration = duration_cast<milliseconds>(now - m_lastTimepoint);
    m_fps = 1000.0f / duration.count();
    m_lastTimepoint = now;

    PipelinePainter::onPaint();
}
