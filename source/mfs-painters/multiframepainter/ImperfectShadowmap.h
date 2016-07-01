#pragma once

#include <glm/fwd.hpp>

#include <globjects/base/ref_ptr.h>

#include "TypeDefinitions.h"

namespace globjects
{
    class Program;
    class Framebuffer;
    class Texture;
}

namespace gloperate
{
    class VertexDrawable;
}

class VPLProcessor;


class ImperfectShadowmap
{
public:
    ImperfectShadowmap();
    ~ImperfectShadowmap();

    void render(const IdDrawablesMap& drawablesMap, const VPLProcessor& vplProcessor, int vplStartIndex, int vplEndIndex, bool scaleISMs, bool pointsOnlyIntoScaledISMs, float tessLevelFactor, float zFar) const;

    globjects::ref_ptr<globjects::Texture> depthBuffer;

protected:

    int m_blurSize;

    globjects::ref_ptr<globjects::Framebuffer> m_fbo;

    globjects::ref_ptr<globjects::Program> m_shadowmapProgram;
};
