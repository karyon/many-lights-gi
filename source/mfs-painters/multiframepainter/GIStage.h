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

class ImperfectShadowmap;
class ModelLoadingStage;
class VPLProcessor;
class ClusteredShading;


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
    globjects::ref_ptr<globjects::Texture> giBlurTempBuffer;
    globjects::ref_ptr<globjects::Texture> giBlurFinalBuffer;
    std::unique_ptr<ImperfectShadowmap> ism;
    std::unique_ptr<VPLProcessor> vplProcessor;
    std::unique_ptr<ClusteredShading> clusteredShading;

    glm::vec3 lightPosition;
    glm::vec3 lightDirection;
    float lightIntensity;

    std::unique_ptr<RasterizationStage> rsmRenderer;

    gloperate::AbstractViewportCapability * viewport;
    gloperate::AbstractProjectionCapability * projection;
    gloperate::AbstractCameraCapability * camera;

    ModelLoadingStage& modelLoadingStage;


protected:
    void render();
    void blur();
    void rebuildGIShader();
    void rebuildBlurShaders();
    void resizeTexture(int width, int height);


    globjects::ref_ptr<globjects::Framebuffer> m_fbo;
    globjects::ref_ptr<globjects::Framebuffer> m_blurTempFbo;
    globjects::ref_ptr<globjects::Framebuffer> m_blurFinalFbo;
    globjects::ref_ptr<globjects::Program> m_giProgram;
    globjects::ref_ptr<gloperate::ScreenAlignedQuad> m_blurXScreenAlignedQuad;
    globjects::ref_ptr<gloperate::ScreenAlignedQuad> m_blurYScreenAlignedQuad;

    std::unique_ptr<gloperate::OrthographicProjectionCapability> m_lightProjection;
    std::unique_ptr<gloperate::AbstractViewportCapability> m_lightViewport;
    std::unique_ptr<gloperate::AbstractCameraCapability> m_lightCamera;
    
    float giIntensityFactor;
    float vplClampingValue;

    int vplStartIndex;
    int vplEndIndex;
    bool scaleISMs;
    bool pointsOnlyIntoScaledISMs;
    float tessLevelFactor;
    bool usePushPull;
    bool enableShadowing;

    float sunCyclePosition;
    float sunCycleSpeed;
    bool moveLight;
    bool showVPLPositions;
    bool useInterleaving;
    bool shuffleLights;

    bool giShaderRebuildRequired;
    bool blurShaderRebuildRequired;
};