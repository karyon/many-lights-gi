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

    usedClustersPerTile = globjects::Texture::createDefault(GL_TEXTURE_3D);
    usedClustersPerTile->setName("used clusters per tile");

    m_usedClusterIDsPerTileProgram = new globjects::Program();
    m_usedClusterIDsPerTileProgram->attach(
        globjects::Shader::fromFile(GL_COMPUTE_SHADER, "data/shaders/used_clusters_per_tile.comp")
    );

    compactUsedClusterIDs = globjects::Texture::createDefault(GL_TEXTURE_1D);
    compactUsedClusterIDs->setName("compact clusters");

    m_compactUsedClusterIDsPerTileProgram = new globjects::Program();
    m_compactUsedClusterIDsPerTileProgram->attach(
        globjects::Shader::fromFile(GL_COMPUTE_SHADER, "data/shaders/compact_used_clusters_per_tile.comp")
    );

    m_atomicCounter = new globjects::Buffer();
    m_atomicCounter->setData(sizeof(gl::GLuint), nullptr, GL_STATIC_DRAW);
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
        clusterIDTexture->bindImageTexture(0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8UI);
        usedClustersPerTile->bindImageTexture(1, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8UI);
        m_usedClusterIDsPerTileProgram->dispatchCompute(viewport.x / 8 + 1, viewport.y / 8 + 1, 1);
    }
    {
        AutoGLPerfCounter c("compactIDs");
        gl::GLuint zero = 0;
        m_atomicCounter->clearData(GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);

        usedClustersPerTile->bindImageTexture(0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8UI);
        compactUsedClusterIDs->bindImageTexture(1, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);
        m_atomicCounter->bindBase(GL_SHADER_STORAGE_BUFFER, 0);
        m_compactUsedClusterIDsPerTileProgram->dispatchCompute(viewport.x / clusterPixelSize / 8 + 1, viewport.y / clusterPixelSize / 8 + 1, 1);
    }
}


void ClusteredShading::resizeTexture(int width, int height)
{
    clusterIDTexture->image2D(0, GL_R8UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
    usedClustersPerTile->image3D(0, GL_R8UI, width / clusterPixelSize + 1, height / clusterPixelSize + 1, numDepthSlices, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
    compactUsedClusterIDs->image1D(0, GL_R32UI, (width / clusterPixelSize + 1) * (height / clusterPixelSize + 1) * numDepthSlices, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
}
