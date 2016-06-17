#include "ClusteredShading.h"

#include <glm/mat4x4.hpp>
#include <glm/integer.hpp>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/boolean.h>

#include <globjects/Program.h>
#include <globjects/Buffer.h>
#include <globjects/Shader.h>
#include <globjects/Texture.h>
#include <globjects/Framebuffer.h>

#include <gloperate/primitives/ScreenAlignedQuad.h>

#include "VPLProcessor.h"
#include "PerfCounter.h"


using namespace gl;


ClusteredShading::ClusteredShading()
{
    clusterIDTexture = globjects::Texture::createDefault(GL_TEXTURE_2D);
    clusterIDTexture->setName("cluster IDs");

    m_clusterIDProgram = new globjects::Program();
    m_clusterIDProgram->attach(
        globjects::Shader::fromFile(GL_COMPUTE_SHADER, "data/shaders/cluster_ids.comp")
    );
}

ClusteredShading::~ClusteredShading()
{

}

void ClusteredShading::process(const VPLProcessor& vplProcessor, const glm::mat4& projection, globjects::ref_ptr<globjects::Texture> depthBuffer, const glm::ivec2& viewport)
{
    {
        AutoGLPerfCounter c("ClusterIDs");
        clusterIDTexture->bindImageTexture(0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);
        m_clusterIDProgram->setUniform("depthSampler", 0);
        m_clusterIDProgram->setUniform("projectionMatrix", projection);
        m_clusterIDProgram->dispatchCompute(viewport.x / 8 + 1, viewport.y / 8 + 1, 1);
    }
}


void ClusteredShading::resizeTexture(int width, int height)
{
    clusterIDTexture->image2D(0, GL_R32UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
}
