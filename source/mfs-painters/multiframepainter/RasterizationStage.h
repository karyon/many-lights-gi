#pragma once

#include <globjects/base/ref_ptr.h>

#include "TypeDefinitions.h"

namespace globjects
{
    class Program;
    class Texture;
    class Framebuffer;
}

namespace gloperate
{
    class AbstractPerspectiveProjectionCapability;
    class AbstractViewportCapability;
    class AbstractCameraCapability;

    class PolygonalDrawable;
}

class NoiseTexture;
class GroundPlane;
class Shadowmap;
class ModelLoadingStage;
class KernelGenerationStage;
class MultiFramePainter;

class RasterizationStage
{
public:
    RasterizationStage(ModelLoadingStage& modelLoadingStage, KernelGenerationStage& kernelGenerationStage);
    ~RasterizationStage();

    void initProperties(MultiFramePainter& painter);

    void initialize();
    void process();

    gloperate::AbstractPerspectiveProjectionCapability * projection;
    gloperate::AbstractViewportCapability * viewport;
    gloperate::AbstractCameraCapability * camera;
    int multiFrameCount;
    bool useDOF;

    int currentFrame;
    globjects::ref_ptr<globjects::Texture> diffuseBuffer;
    globjects::ref_ptr<globjects::Texture> specularBuffer;
    globjects::ref_ptr<globjects::Texture> normalBuffer;
    globjects::ref_ptr<globjects::Texture> faceNormalBuffer;
    globjects::ref_ptr<globjects::Texture> worldPosBuffer;
    globjects::ref_ptr<globjects::Texture> depthBuffer;


protected:

    void resizeTextures(int width, int height);
    static void setupGLState();
    void render();
    void zPrepass();

    std::unique_ptr<Shadowmap> m_shadowmap;
    std::unique_ptr<GroundPlane> m_groundPlane;

    globjects::ref_ptr<globjects::Framebuffer> m_fbo;
    globjects::ref_ptr<globjects::Program> m_program;
    globjects::ref_ptr<globjects::Program> m_zOnlyProgram;

    ModelLoadingStage& m_modelLoadingStage;
    KernelGenerationStage& m_kernelGenerationStage;
    const PresetInformation& m_presetInformation;
    glm::vec3 m_lightPosition;
    glm::vec3 m_lightDirection;
};
