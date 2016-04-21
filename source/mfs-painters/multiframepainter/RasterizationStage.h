#pragma once

#include <globjects/base/ref_ptr.h>

#include <glkernel/Kernel.h>

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
class PostprocessingStage;
class FrameAccumulationStage;
class BlitStage;

class RasterizationStage
{
public:
    RasterizationStage(ModelLoadingStage& modelLoadingStage, KernelGenerationStage& kernelGenerationStage, PostprocessingStage& postProcessingStage, FrameAccumulationStage& frameAccumulationStage, BlitStage& blitStage);
    ~RasterizationStage();

    void initialize();
    void process();

    gloperate::AbstractPerspectiveProjectionCapability * projection;
    gloperate::AbstractViewportCapability * viewport;
    gloperate::AbstractCameraCapability * camera;
    int multiFrameCount;
    bool useDOF;
    glm::vec3 lightPosition;

    int currentFrame;
    globjects::ref_ptr<globjects::Texture> color;
    globjects::ref_ptr<globjects::Texture> normal;
    globjects::ref_ptr<globjects::Texture> worldPos;
    globjects::ref_ptr<globjects::Texture> depth;


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
    PostprocessingStage& m_postProcessingStage;
    FrameAccumulationStage& m_frameAccumulationStage;
    BlitStage& m_blitStage;
    PresetInformation m_presetInformation;
};
