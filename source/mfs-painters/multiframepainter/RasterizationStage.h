#pragma once

#include <globjects/base/ref_ptr.h>

#include <gloperate/pipeline/AbstractStage.h>
#include <gloperate/pipeline/InputSlot.h>
#include <gloperate/pipeline/Data.h>

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

class RasterizationStage : public gloperate::AbstractStage
{
public:
    RasterizationStage(ModelLoadingStage& modelLoadingStage, KernelGenerationStage& kernelGenerationStage);

    virtual void initialize() override;

    gloperate::InputSlot<gloperate::AbstractPerspectiveProjectionCapability *> projection;
    gloperate::InputSlot<gloperate::AbstractViewportCapability *> viewport;
    gloperate::InputSlot<gloperate::AbstractCameraCapability *> camera;
    gloperate::InputSlot<int> multiFrameCount;
    gloperate::InputSlot<bool> useReflections;
    gloperate::InputSlot<bool> useDOF;
    gloperate::InputSlot<glm::vec3> lightPosition;

    gloperate::Data<int> currentFrame;
    gloperate::Data<globjects::ref_ptr<globjects::Texture>> color;
    gloperate::Data<globjects::ref_ptr<globjects::Texture>> normal;
    gloperate::Data<globjects::ref_ptr<globjects::Texture>> worldPos;
    gloperate::Data<globjects::ref_ptr<globjects::Texture>> reflectMask;
    gloperate::Data<globjects::ref_ptr<globjects::Texture>> depth;


protected:
    virtual void process() override;

    void resizeTextures(int width, int height);
    static void setupGLState();
    void setupMasksTexture();
    void render();
    void zPrepass();

    std::unique_ptr<Shadowmap> m_shadowmap;
    std::unique_ptr<GroundPlane> m_groundPlane;

    globjects::ref_ptr<globjects::Framebuffer> m_fbo;
    globjects::ref_ptr<globjects::Program> m_program;
    globjects::ref_ptr<globjects::Program> m_zOnlyProgram;

    globjects::ref_ptr<globjects::Texture> m_masksTexture;
    std::unique_ptr<NoiseTexture> m_noiseTexture;
    ModelLoadingStage& m_modelLoadingStage;
    KernelGenerationStage& m_kernelGenerationStage;
    PresetInformation m_presetInformation;
};
