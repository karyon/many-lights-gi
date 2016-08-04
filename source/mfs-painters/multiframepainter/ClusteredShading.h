#pragma once

#include <glm/fwd.hpp>

#include <globjects/base/ref_ptr.h>

namespace globjects
{
    class Buffer;
    class Program;
    class Texture;
    class Framebuffer;
}

namespace gloperate
{
    class ScreenAlignedQuad;
}

class RasterizationStage;
class VPLProcessor;


class ClusteredShading
{
public:
    ClusteredShading();
    ~ClusteredShading();

    void process(
        const VPLProcessor& vplProcessor,
        const glm::mat4& view,
        const glm::mat4& projection,
        const glm::ivec2& viewport,
        float zFar,
        int vplStartIndex,
        int vplEndIndex,
        globjects::ref_ptr<globjects::Texture> depthBuffer,
        const globjects::ref_ptr<globjects::Buffer> vplBuffer);
    void resizeTexture(int width, int height);

    globjects::ref_ptr<globjects::Buffer> vplBuffer;
    globjects::ref_ptr<globjects::Texture> clusterIDs;
    globjects::ref_ptr<globjects::Texture> usedClustersPerTile;
    globjects::ref_ptr<globjects::Texture> compactUsedClusterIDs;
    globjects::ref_ptr<globjects::Texture> normalToCompactIDs;
    globjects::ref_ptr<globjects::Buffer> lightListsBuffer;
    globjects::ref_ptr<globjects::Texture> lightLists;
    globjects::ref_ptr<globjects::Texture> clusterCorners;

private:
    int m_numClustersX;
    int m_numClustersY;
    int m_numClusters;
    globjects::ref_ptr<globjects::Program> m_clusterIDProgram;
    globjects::ref_ptr<globjects::Program> m_usedClusterIDsPerTileProgram;
    globjects::ref_ptr<globjects::Program> m_compactUsedClusterIDsPerTileProgram;
    globjects::ref_ptr<globjects::Program> m_lightListsProgram;

    globjects::ref_ptr<globjects::Buffer> m_atomicCounter;
};
