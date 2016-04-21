#pragma once

#include <glkernel/Kernel.h>

#include "TypeDefinitions.h"

namespace globjects
{
    class Framebuffer;
    class Texture;
}

class KernelGenerationStage
{
public:
    KernelGenerationStage();

    void initialize();
    void process(int multiFrameCount);

    glkernel::kernel3 getSSAOKernel() const;
    std::vector<glm::vec3> getSSAONoise() const;

    glkernel::kernel2 antiAliasingKernel;
    glkernel::kernel2 depthOfFieldKernel;
    glkernel::kernel2 shadowKernel;
    glkernel::kernel3 reflectionKernel;
};
