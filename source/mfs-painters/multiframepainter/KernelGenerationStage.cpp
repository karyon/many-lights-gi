#include "KernelGenerationStage.h"

#include <glm/gtc/random.hpp>

#include <glkernel/Kernel.h>
#include <glkernel/sample.h>
#include <glkernel/scale.h>
#include <glkernel/sort.hpp>
#include <glkernel/shuffle.hpp>

namespace
{
    std::vector<glm::vec3> generateSSAONoise(const unsigned int size)
    {
        auto kernel = std::vector<glm::vec3>();

        for (auto y = 0u; y < size; ++y)
        {
            for (auto x = 0u; x < size; ++x)
            {
                auto c = glm::circularRand(1.f);
                auto v = glm::vec3(c.x, c.y, 0.0f);

                kernel.push_back(v);
            }
        }

        return kernel;
    }

    const unsigned int s_ssaoKernelSize = 16;
    const unsigned int s_ssaoNoiseSize = 128;
}

KernelGenerationStage::KernelGenerationStage()
{
    addInput("multiFrameCount", multiFrameCount);

    addOutput("antiAliasingKernel", antiAliasingKernel);
    addOutput("depthOfFieldKernel", depthOfFieldKernel);
    addOutput("shadowKernel", shadowKernel);
    addOutput("ssaoKernel", ssaoKernel);
    addOutput("reflectionKernel", reflectionKernel);
    addOutput("ssaoNoise", ssaoNoise);
    addOutput("ssaoKernelSize", ssaoKernelSize);
    addOutput("ssaoNoiseSize", ssaoNoiseSize);

    alwaysProcess(true);
}

void KernelGenerationStage::initialize()
{
    ssaoNoise.data() = generateSSAONoise(s_ssaoNoiseSize);
    ssaoNoiseSize.data() = s_ssaoNoiseSize;
    ssaoKernelSize.data() = s_ssaoKernelSize;
}

void KernelGenerationStage::process()
{
    if (multiFrameCount.hasChanged())
    {
        auto& aaSamples = antiAliasingKernel.data();
        aaSamples = { static_cast<uint16_t>(multiFrameCount.data()) };
        glkernel::sample::poisson_square(aaSamples);
        glkernel::scale::range(aaSamples, -.5f, .5f);
        glkernel::shuffle::random(aaSamples, 1);

        auto& dofSamples = depthOfFieldKernel.data();
        dofSamples = { static_cast<uint16_t>(multiFrameCount.data()) };
        glkernel::sample::poisson_square(dofSamples);
        glkernel::scale::range(dofSamples, -1.f, 1.f);
        glkernel::sort::distance(dofSamples, { 0.f, 0.f });

        auto& shadowSamples = shadowKernel.data();
        shadowSamples = { static_cast<uint16_t>(multiFrameCount.data()) };
        glkernel::sample::poisson_square(shadowSamples);
        glkernel::scale::range(shadowSamples, -1.f, 1.f);
        glkernel::sort::distance(shadowSamples, { 0.f, 0.f });

        auto& reflectionSamples = reflectionKernel.data();
        reflectionSamples = { static_cast<uint16_t>(multiFrameCount.data()) };
        glkernel::sample::stratified(reflectionSamples);
        glkernel::scale::range(reflectionSamples, -1.f, 1.f);
        glkernel::sort::distance(reflectionSamples, { 0.f, 0.f, 0.f });
        reflectionSamples[0] = glm::vec3(0.0f);
    }

    auto& ssaoSamples = ssaoKernel.data();
    ssaoSamples = glkernel::kernel3{ static_cast<uint16_t>(s_ssaoKernelSize) };
    glkernel::sample::best_candidate(ssaoSamples);
    glkernel::scale::range(ssaoSamples, -1.0f, 1.0f);
    for (auto& elem : ssaoSamples)
    {
        elem.z = glm::abs(elem.z);
        elem.z = std::max(0.1f, elem.z);
        elem = glm::normalize(elem);
    }

    if (multiFrameCount.hasChanged())
        invalidateOutputs();
    else
        ssaoKernel.invalidate();
}
