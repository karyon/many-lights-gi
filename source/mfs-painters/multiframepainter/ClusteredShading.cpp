#include "ClusteredShading.h"

#include <glm/mat4x4.hpp>
#include <glm/integer.hpp>
#include <glm/gtc/matrix_inverse.hpp>

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
    const int maxVPLCount = 1024;
}


ClusteredShading::ClusteredShading()
{
    clusterIDs = globjects::Texture::createDefault(GL_TEXTURE_2D);
    clusterIDs->setName("cluster IDs");

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
    normalToCompactIDs = globjects::Texture::createDefault(GL_TEXTURE_3D);
    normalToCompactIDs->setName("normalToCompactIDs");

    m_compactUsedClusterIDsPerTileProgram = new globjects::Program();
    m_compactUsedClusterIDsPerTileProgram->attach(
        globjects::Shader::fromFile(GL_COMPUTE_SHADER, "data/shaders/compact_used_clusters_per_tile.comp")
    );

    m_atomicCounter = new globjects::Buffer();
    m_atomicCounter->setName("atomic counter");
    m_atomicCounter->setData(sizeof(gl::GLuint), nullptr, GL_STATIC_DRAW);

    m_lightListsProgram = new globjects::Program();
    m_lightListsProgram->attach(
        globjects::Shader::fromFile(GL_COMPUTE_SHADER, "data/shaders/light_lists.comp")
    );

    //lightListsBuffer = new globjects::Buffer();
    lightLists = globjects::Texture::createDefault(GL_TEXTURE_2D);
    //lightLists->texBuffer(GL_R16UI, lightListsBuffer);
    lightLists->setName("light lists");

    clusterCorners = globjects::Texture::createDefault(GL_TEXTURE_2D);
    clusterCorners->setName("clusterCorners");
}

ClusteredShading::~ClusteredShading()
{

}

void ClusteredShading::process(
    const VPLProcessor& vplProcessor,
    const glm::mat4& view,
    const glm::mat4& projection,
    const glm::ivec2& viewport,
    int vplStartIndex,
    int vplEndIndex,
    globjects::ref_ptr<globjects::Texture> depthBuffer,
    const globjects::ref_ptr<globjects::Buffer> vplBuffer)
{

    {
        AutoGLPerfCounter c("ClusterIDs");
        depthBuffer->bindActive(0);
        clusterIDs->bindImageTexture(0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8UI);
        m_clusterIDProgram->setUniform("depthSampler", 0);
        m_clusterIDProgram->setUniform("projectionMatrix", projection);
        m_clusterIDProgram->dispatchCompute(viewport.x / 8 + 1, viewport.y / 8 + 1, 1);
    }
    {
        AutoGLPerfCounter c("uniqueIDsPerTile");
        clusterIDs->bindImageTexture(0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8UI);
        usedClustersPerTile->clearImage(0, GL_RED_INTEGER, GL_UNSIGNED_INT, glm::uvec4(0));
        usedClustersPerTile->bindImageTexture(1, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8UI);
        m_usedClusterIDsPerTileProgram->dispatchCompute(viewport.x / 8 + 1, viewport.y / 8 + 1, 1);
    }
    {
        AutoGLPerfCounter c("compactIDs");
        gl::GLuint zero = 0;
        m_atomicCounter->clearData(GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);

        usedClustersPerTile->bindImageTexture(0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8UI);
        compactUsedClusterIDs->bindImageTexture(1, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);
        normalToCompactIDs->bindImageTexture(2, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16UI);
        m_atomicCounter->bindBase(GL_SHADER_STORAGE_BUFFER, 0);
        m_compactUsedClusterIDsPerTileProgram->dispatchCompute(m_numClustersX / 8 + 1, m_numClustersY / 8 + 1, 1);
    }
    {
        AutoGLPerfCounter c("light lists");
        compactUsedClusterIDs->bindImageTexture(0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
        lightLists->clearImage(0, GL_RED_INTEGER, GL_UNSIGNED_INT, glm::uvec4(0));
        lightLists->bindImageTexture(1, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16UI);
        clusterCorners->bindImageTexture(2, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        m_atomicCounter->bindBase(GL_SHADER_STORAGE_BUFFER, 0);
        //lightLists->bindImageTexture(1, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16UI); //uncomment this line to get the crash
        //lightListsBuffer->bindBase(GL_SHADER_STORAGE_BUFFER, 1);
        m_lightListsProgram->setUniform("viewport", viewport);
        m_lightListsProgram->setUniform("projectionInverseMatrix", glm::inverse(projection));
        m_lightListsProgram->setUniform("viewInverseMatrix", glm::inverse(view));
        m_lightListsProgram->setUniform("vplStartIndex", vplStartIndex);
        m_lightListsProgram->setUniform("vplEndIndex", vplEndIndex);
        m_lightListsProgram->dispatchCompute(m_numClusters / 32 + 1, 1, 1);
    }
}


void ClusteredShading::resizeTexture(int width, int height)
{
    m_numClustersX = int(glm::ceil(float(width) / clusterPixelSize));
    m_numClustersY = int(glm::ceil(float(height) / clusterPixelSize));
    m_numClusters = m_numClustersX * m_numClustersY * numDepthSlices;

    clusterIDs->image2D(0, GL_R8UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
    usedClustersPerTile->image3D(0, GL_R8UI, m_numClustersX, m_numClustersY, numDepthSlices, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
    compactUsedClusterIDs->image1D(0, GL_R32UI, m_numClusters, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
    normalToCompactIDs->image3D(0, GL_R16UI, m_numClustersX, m_numClustersY, numDepthSlices, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
    // TODO memory usage. m_numClusters is theoretical worst case.
    lightLists->image2D(0, GL_R16UI, m_numClusters, maxVPLCount, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
    clusterCorners->image2D(0, GL_RGBA32F, m_numClusters, 8, 0, GL_RGBA, GL_FLOAT, nullptr);
    //lightListsBuffer->setData(m_numClusters * maxVPLCount * sizeof(short), nullptr, GL_STATIC_DRAW);
}
