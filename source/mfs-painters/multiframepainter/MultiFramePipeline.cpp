#include "MultiFramePipeline.h"

#include "ModelLoadingStage.h"
#include "KernelGenerationStage.h"
#include "RasterizationStage.h"
#include "PostprocessingStage.h"
#include "FrameAccumulationStage.h"
#include "BlitStage.h"


MultiFramePipeline::MultiFramePipeline(gloperate::ResourceManager& resourceManager)
: gloperate::AbstractPipeline("MultiframeSampling")
, resourceManager(nullptr)
, multiFrameCount(64)
, preset(Preset::CrytekSponza)
, useReflections(false)
{
    auto modelLoadingStage = new ModelLoadingStage();
    auto kernelGenerationStage = new KernelGenerationStage();
    auto rasterizationStage = new RasterizationStage(*modelLoadingStage, *kernelGenerationStage);
    auto postprocessingStage = new PostprocessingStage(*kernelGenerationStage);
    auto frameAccumulationStage = new FrameAccumulationStage();
    auto blitStage = new BlitStage();

    modelLoadingStage->resourceManager = &resourceManager;
    modelLoadingStage->preset = preset;

    rasterizationStage->projection = projection;
    rasterizationStage->camera = camera;
    rasterizationStage->viewport = viewport;
    rasterizationStage->multiFrameCount = multiFrameCount;
    rasterizationStage->useReflections = useReflections;
    rasterizationStage->useDOF = useDOF;

    postprocessingStage->viewport = viewport;
    postprocessingStage->camera = camera;
    postprocessingStage->projection = projection;
    postprocessingStage->useReflections = useReflections;
    postprocessingStage->currentFrame = rasterizationStage->currentFrame;
    postprocessingStage->color = rasterizationStage->color;
    postprocessingStage->normal = rasterizationStage->normal;
    postprocessingStage->depth = rasterizationStage->depth;
    postprocessingStage->worldPos = rasterizationStage->worldPos;
    postprocessingStage->reflectMask = rasterizationStage->reflectMask;

    frameAccumulationStage->viewport = viewport;
    frameAccumulationStage->currentFrame = rasterizationStage->currentFrame;
    frameAccumulationStage->frame = postprocessingStage->postprocessedFrame;
    frameAccumulationStage->depth = rasterizationStage->depth;

    blitStage->viewport = viewport;
    blitStage->accumulation = frameAccumulationStage->accumulation;
    blitStage->depth = rasterizationStage->depth;

    addStages(rasterizationStage, postprocessingStage, frameAccumulationStage, blitStage);
}
