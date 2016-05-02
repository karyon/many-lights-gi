#pragma once

#include <memory>

#include <glkernel/Kernel.h>

#include <globjects/base/ref_ptr.h>

#include "TypeDefinitions.h"

namespace globjects
{
    class Framebuffer;
    class Texture;
    class Program;
}

namespace gloperate
{
    class AbstractViewportCapability;
    class AbstractPerspectiveProjectionCapability;
    class AbstractCameraCapability;

    class ScreenAlignedQuad;
}

class Shadowmap;
class ModelLoadingStage;
class MultiFramePainter;

class DeferredShadingStage
{
public:
    DeferredShadingStage(ModelLoadingStage& modelLoadingStage);
    ~DeferredShadingStage();
    
    void initProperties(MultiFramePainter& painter);

    void initialize();
    void process();

    gloperate::AbstractPerspectiveProjectionCapability * projection;
    gloperate::AbstractViewportCapability * viewport;
    gloperate::AbstractCameraCapability * camera;

    globjects::ref_ptr<globjects::Texture> diffuseBuffer;
    globjects::ref_ptr<globjects::Texture> specularBuffer;
    globjects::ref_ptr<globjects::Texture> giBuffer;
    globjects::ref_ptr<globjects::Texture> faceNormalBuffer;
    globjects::ref_ptr<globjects::Texture> normalBuffer;
    globjects::ref_ptr<globjects::Texture> depthBuffer;

    globjects::ref_ptr<globjects::Texture> shadedFrame;

protected:
    void resizeTexture(int width, int height);

    globjects::ref_ptr<globjects::Framebuffer> m_fbo;
    globjects::ref_ptr<gloperate::ScreenAlignedQuad> m_screenAlignedQuad;
    globjects::ref_ptr<globjects::Program> m_program;
    glm::vec3 m_lightPosition;
    glm::vec3 m_lightDirection;
    std::unique_ptr<Shadowmap> m_shadowmap;
    ModelLoadingStage& m_modelLoadingStage;
};
