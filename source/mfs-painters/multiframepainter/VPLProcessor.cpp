#include "VPLProcessor.h"

#include <random>
#include <numeric>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glbinding/gl/enum.h>

#include <globjects/Program.h>
#include <globjects/Buffer.h>
#include <globjects/Shader.h>

#include <gloperate/painter/AbstractCameraCapability.h>
#include <gloperate/painter/AbstractProjectionCapability.h>

#include "RasterizationStage.h"


using namespace gl;

namespace
{
    const int maxVPLCount = 1024;
}

struct packedVPL {
    glm::vec4 positionNormal;
    // no color, ISMs / culling doesn't need it
};

struct vpl {
    glm::vec3 position;
    float padding1;
    glm::vec3 normal;
    float padding2;
    glm::vec3 color;
    float padding3;
};

VPLProcessor::VPLProcessor()
{
    m_program = new globjects::Program();

    m_program->attach(
        globjects::Shader::fromFile(GL_COMPUTE_SHADER, "data/shaders/gi/vpl_processor.comp")
    );

    vplBuffer = new globjects::Buffer();
    vplBuffer->setData(sizeof(vpl) * maxVPLCount, nullptr, GL_STATIC_DRAW);

    packedVplBuffer = new globjects::Buffer();
    packedVplBuffer->setData(sizeof(packedVPL) * maxVPLCount, nullptr, GL_STATIC_DRAW);


    std::vector<int> v(maxVPLCount);
    std::iota(v.begin(), v.end(), 0);

    std::mt19937 g(1979982); // fixed seed to be reproducible
    std::shuffle(v.begin(), v.end(), g);

    m_shuffledIndicesBuffer = new globjects::Buffer();
    m_shuffledIndicesBuffer->setData(sizeof(int) * v.size(), v.data(), GL_STATIC_DRAW);
}

VPLProcessor::~VPLProcessor()
{

}

void VPLProcessor::process(const RasterizationStage& rsmRenderer, float lightIntensity, bool shuffleLights)
{

    auto shadowBias = glm::mat4(
        0.5f, 0.0f, 0.0f, 0.0f
        , 0.0f, 0.5f, 0.0f, 0.0f
        , 0.0f, 0.0f, 0.5f, 0.0f
        , 0.5f, 0.5f, 0.5f, 1.0f);

    biasedShadowTransform = shadowBias * rsmRenderer.projection->projection() * rsmRenderer.camera->view();

    rsmRenderer.diffuseBuffer->bindActive(0);
    rsmRenderer.faceNormalBuffer->bindActive(1);
    rsmRenderer.depthBuffer->bindActive(2);

    m_program->setUniform("rsmDiffuseSampler", 0);
    m_program->setUniform("rsmNormalSampler", 1);
    m_program->setUniform("rsmDepthSampler", 2);
    m_program->setUniform("biasedLightViewProjectionInverseMatrix", glm::inverse(biasedShadowTransform));
    m_program->setUniform("lightIntensity", lightIntensity);
    m_program->setUniform("shuffleLights", shuffleLights);

    vplBuffer->bindBase(GL_SHADER_STORAGE_BUFFER, 0);
    packedVplBuffer->bindBase(GL_SHADER_STORAGE_BUFFER, 1);
    m_shuffledIndicesBuffer->bindBase(GL_UNIFORM_BUFFER, 1);

    int localSize = 64; // must match shader
    m_program->dispatchCompute(maxVPLCount / localSize, 1, 1);
}
