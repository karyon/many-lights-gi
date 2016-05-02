#include <memory>

#include <glm/fwd.hpp>

#include <globjects/base/ref_ptr.h>

#include "RasterizationStage.h"
#include "ModelLoadingStage.h"


namespace globjects
{
    class Program;
    class Framebuffer;
    class Texture;
}

namespace gloperate
{
    class ScreenAlignedQuad;
    class AbstractPerspectiveProjectionCapability;
    class AbstractViewportCapability;
    class AbstractCameraCapability;
}

class ModelLoadingStage;
class KernelGenerationStage;


class GIStage
{
public:
    GIStage(ModelLoadingStage& modelLoadingStage, KernelGenerationStage& kernelGenerationStage);
    ~GIStage();

    void initialize();
    void process();

    globjects::ref_ptr<globjects::Texture> faceNormalBuffer;
    globjects::ref_ptr<globjects::Texture> depthBuffer;

    globjects::ref_ptr<globjects::Texture> giBuffer;

    gloperate::AbstractViewportCapability * viewport;
    gloperate::AbstractPerspectiveProjectionCapability * projection;
    gloperate::AbstractCameraCapability * camera;

    ModelLoadingStage& modelLoadingStage;

protected:
    void resizeTexture(int width, int height);

    std::unique_ptr<RasterizationStage> m_rsmRenderer;

    globjects::ref_ptr<globjects::Framebuffer> m_fbo;
    globjects::ref_ptr<gloperate::ScreenAlignedQuad> m_screenAlignedQuad;
    globjects::ref_ptr<globjects::Program> m_program;
    glm::vec3 m_lightPosition;
    glm::vec3 m_lightDirection;

    std::unique_ptr<gloperate::AbstractPerspectiveProjectionCapability> m_lightProjection;
    std::unique_ptr<gloperate::AbstractViewportCapability> m_lightViewport;
    std::unique_ptr<gloperate::AbstractCameraCapability> m_lightCamera;
};