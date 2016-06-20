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

    void process(const VPLProcessor& vplProcessor, const glm::mat4& projection, globjects::ref_ptr<globjects::Texture> depthBuffer, const glm::ivec2& viewport);

    void resizeTexture(int width, int height);

    globjects::ref_ptr<globjects::Buffer> vplBuffer;
    globjects::ref_ptr<globjects::Texture> usedClusterIDsPerTile;
    globjects::ref_ptr<globjects::Texture> clusterIDTexture;

private:
    globjects::ref_ptr<globjects::Program> m_clusterIDProgram;
    globjects::ref_ptr<globjects::Program> m_usedClusterIDsPerTileProgram;
};
