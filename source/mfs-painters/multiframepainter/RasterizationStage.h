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
    class AbstractProjectionCapability;
    class AbstractViewportCapability;
    class AbstractCameraCapability;

    class PolygonalDrawable;
}

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
    void loadPreset(const PresetInformation& preset);
    void process();


    gloperate::AbstractProjectionCapability * projection;
    gloperate::AbstractViewportCapability * viewport;
    gloperate::AbstractCameraCapability * camera;
    bool useDOF;

    int currentFrame;
    globjects::ref_ptr<globjects::Texture> diffuseBuffer;
    globjects::ref_ptr<globjects::Texture> specularBuffer;
    globjects::ref_ptr<globjects::Texture> normalBuffer;
    globjects::ref_ptr<globjects::Texture> faceNormalBuffer;
    globjects::ref_ptr<globjects::Texture> depthBuffer;


protected:

    void resizeTextures(int width, int height);
    static void setupGLState();
    void render();
    void zPrepass();

    globjects::ref_ptr<globjects::Framebuffer> m_fbo;
    globjects::ref_ptr<globjects::Program> m_program;
    globjects::ref_ptr<globjects::Program> m_zOnlyProgram;

    float m_focalPoint;
    float m_focalDist;
    BumpType m_bumpType;

    ModelLoadingStage& m_modelLoadingStage;
    KernelGenerationStage& m_kernelGenerationStage;
};
