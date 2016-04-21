#include "KernelGenerationStage.h"

#include <glm/gtc/random.hpp>

#include <glkernel/Kernel.h>
#include <glkernel/sample.h>
#include <glkernel/scale.h>
#include <glkernel/sort.hpp>
#include <glkernel/shuffle.hpp>

namespace
{
    const unsigned int s_ssaoKernelSize = 16;
    const unsigned int s_ssaoNoiseSize = 128;
}

KernelGenerationStage::KernelGenerationStage()
{
}

void KernelGenerationStage::initialize()
{
}

void KernelGenerationStage::process(int multiFrameCount)
{
    auto& aaSamples = antiAliasingKernel;
    aaSamples = { static_cast<uint16_t>(multiFrameCount) };
    glkernel::sample::poisson_square(aaSamples);
    glkernel::scale::range(aaSamples, -.5f, .5f);
    glkernel::shuffle::random(aaSamples, 1);

    auto& dofSamples = depthOfFieldKernel;
    dofSamples = { static_cast<uint16_t>(multiFrameCount) };
    glkernel::sample::poisson_square(dofSamples);
    glkernel::scale::range(dofSamples, -1.f, 1.f);
    glkernel::sort::distance(dofSamples, { 0.f, 0.f });

    auto& shadowSamples = shadowKernel;
    shadowSamples = { static_cast<uint16_t>(multiFrameCount) };
    glkernel::sample::poisson_square(shadowSamples);
    glkernel::scale::range(shadowSamples, -1.f, 1.f);
    glkernel::sort::distance(shadowSamples, { 0.f, 0.f });

    auto& reflectionSamples = reflectionKernel;
    reflectionSamples = { static_cast<uint16_t>(multiFrameCount) };
    glkernel::sample::stratified(reflectionSamples);
    glkernel::scale::range(reflectionSamples, -1.f, 1.f);
    glkernel::sort::distance(reflectionSamples, { 0.f, 0.f, 0.f });
    reflectionSamples[0] = glm::vec3(0.0f);
}



glkernel::kernel3 KernelGenerationStage::getSSAOKernel() const
{
    glkernel::kernel3 ssaoSamples = glkernel::kernel3{ static_cast<uint16_t>(s_ssaoKernelSize) };
    glkernel::sample::best_candidate(ssaoSamples);
    glkernel::scale::range(ssaoSamples, -1.0f, 1.0f);
    for (auto& elem : ssaoSamples)
    {
        elem.z = glm::abs(elem.z);
        elem.z = std::max(0.1f, elem.z);
        elem = glm::normalize(elem);
    }
    return ssaoSamples;
}

std::vector<glm::vec3> KernelGenerationStage::getSSAONoise() const
{
    auto kernel = std::vector<glm::vec3>();

    for (auto y = 0u; y < s_ssaoNoiseSize; ++y)
    {
        for (auto x = 0u; x < s_ssaoNoiseSize; ++x)
        {
            auto c = glm::circularRand(1.f);
            auto v = glm::vec3(c.x, c.y, 0.0f);

            kernel.push_back(v);
        }
    }

    return kernel;
}

