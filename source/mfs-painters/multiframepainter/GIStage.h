#include <memory>

#include <glm/glm.hpp>

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
    class OrthographicProjectionCapability;
    class AbstractViewportCapability;
    class AbstractCameraCapability;
}

class Shadowmap;
class ImperfectShadowmap;
class ModelLoadingStage;
class VPLProcessor;


class GIStage
{
public:
    GIStage(ModelLoadingStage& modelLoadingStage, KernelGenerationStage& kernelGenerationStage);
    ~GIStage();

    void initProperties(MultiFramePainter& painter);
    void initialize();
    void process();

    globjects::ref_ptr<globjects::Texture> faceNormalBuffer;
    globjects::ref_ptr<globjects::Texture> depthBuffer;

    globjects::ref_ptr<globjects::Texture> giBuffer;
    std::unique_ptr<Shadowmap> shadowmap;
    std::unique_ptr<ImperfectShadowmap> ism;
    glm::vec3 lightPosition;
    glm::vec3 lightDirection;

    std::unique_ptr<RasterizationStage> rsmRenderer;

    gloperate::AbstractViewportCapability * viewport;
    gloperate::AbstractProjectionCapability * projection;
    gloperate::AbstractCameraCapability * camera;

    ModelLoadingStage& modelLoadingStage;

    std::unique_ptr<VPLProcessor> vplProcessor;

protected:
    void resizeTexture(int width, int height);


    globjects::ref_ptr<globjects::Framebuffer> m_fbo;
    globjects::ref_ptr<gloperate::ScreenAlignedQuad> m_screenAlignedQuad;
    globjects::ref_ptr<globjects::Program> m_program;

    std::unique_ptr<gloperate::OrthographicProjectionCapability> m_lightProjection;
    std::unique_ptr<gloperate::AbstractViewportCapability> m_lightViewport;
    std::unique_ptr<gloperate::AbstractCameraCapability> m_lightCamera;
};