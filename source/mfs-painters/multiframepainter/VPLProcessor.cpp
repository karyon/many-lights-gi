#include "VPLProcessor.h"

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


struct vpl {
    glm::vec4 position;
    glm::vec4 normal;
    glm::vec4 color;
};

VPLProcessor::VPLProcessor(const RasterizationStage& rsmRenderer)
: m_rsmRenderer(rsmRenderer)
{
    m_program = new globjects::Program();

    m_program->attach(
        globjects::Shader::fromFile(GL_COMPUTE_SHADER, "data/shaders/vpl_processor.comp")
    );

    vplBuffer = new globjects::Buffer();

    vplBuffer->setData(sizeof(vpl) * 256, nullptr, GL_STATIC_DRAW);
}

VPLProcessor::~VPLProcessor()
{

}

void VPLProcessor::process()
{

    auto shadowBias = glm::mat4(
        0.5f, 0.0f, 0.0f, 0.0f
        , 0.0f, 0.5f, 0.0f, 0.0f
        , 0.0f, 0.0f, 0.5f, 0.0f
        , 0.5f, 0.5f, 0.5f, 1.0f);

    biasedShadowTransform = shadowBias * m_rsmRenderer.projection->projection() * m_rsmRenderer.camera->view();

    m_rsmRenderer.diffuseBuffer->bindActive(0);
    m_rsmRenderer.faceNormalBuffer->bindActive(1);
    m_rsmRenderer.depthBuffer->bindActive(2);

    m_program->setUniform("rsmDiffuseSampler", 0);
    m_program->setUniform("rsmNormalSampler", 1);
    m_program->setUniform("rsmDepthSampler", 2);
    m_program->setUniform("biasedLightViewProjectionInverseMatrix", glm::inverse(biasedShadowTransform));

    vplBuffer->bindBase(GL_SHADER_STORAGE_BUFFER, 0);

    m_program->dispatchCompute(4, 1, 1);
}