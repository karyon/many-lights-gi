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

namespace
{
    const int clusterPixelSize = 64;
    const int numDepthSlices = 16;
}


ClusteredShading::ClusteredShading()
{
    clusterIDTexture = globjects::Texture::createDefault(GL_TEXTURE_2D);
    clusterIDTexture->setName("cluster IDs");

    m_clusterIDProgram = new globjects::Program();
    m_clusterIDProgram->attach(
        globjects::Shader::fromFile(GL_COMPUTE_SHADER, "data/shaders/cluster_ids.comp")
    );

    usedClusterIDsPerTile = globjects::Texture::createDefault(GL_TEXTURE_3D);
    usedClusterIDsPerTile->setName("used clusters per tile");

    m_usedClusterIDsPerTileProgram = new globjects::Program();
    m_usedClusterIDsPerTileProgram->attach(
        globjects::Shader::fromFile(GL_COMPUTE_SHADER, "data/shaders/used_clusters_per_tile.comp")
    );
}

ClusteredShading::~ClusteredShading()
{

}

void ClusteredShading::process(const VPLProcessor& vplProcessor, const glm::mat4& projection, globjects::ref_ptr<globjects::Texture> depthBuffer, const glm::ivec2& viewport)
{
    {
        AutoGLPerfCounter c("ClusterIDs");
        depthBuffer->bindActive(0);
        clusterIDTexture->bindImageTexture(0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8UI);
        m_clusterIDProgram->setUniform("depthSampler", 0);
        m_clusterIDProgram->setUniform("projectionMatrix", projection);
        m_clusterIDProgram->dispatchCompute(viewport.x / 8 + 1, viewport.y / 8 + 1, 1);
    }
    {
        AutoGLPerfCounter c("uniqueIDsPerTile");
        clusterIDTexture->bindImageTexture(0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
        usedClusterIDsPerTile->bindImageTexture(1, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8UI);
        m_usedClusterIDsPerTileProgram->dispatchCompute(viewport.x / 8 + 1, viewport.y / 8 + 1, 1);
    }
}


void ClusteredShading::resizeTexture(int width, int height)
{
    clusterIDTexture->image2D(0, GL_R8UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
    usedClusterIDsPerTile->image3D(0, GL_R8UI, width / clusterPixelSize + 1, height / clusterPixelSize + 1, numDepthSlices, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
}
