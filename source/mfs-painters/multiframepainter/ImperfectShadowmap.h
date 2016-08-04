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

    void process(
        const IdDrawablesMap& drawablesMap,
        const VPLProcessor& vplProcessor,
        int vplStartIndex,
        int vplEndIndex,
        bool scaleISMs,
        bool pointsOnlyIntoScaledISMs,
        float tessLevelFactor,
        bool usePushPull,
        float zFar) const;

    globjects::ref_ptr<globjects::Texture> depthBuffer;
    globjects::ref_ptr<globjects::Texture> softrenderBuffer;
    globjects::ref_ptr<globjects::Texture> pullBuffer;
    globjects::ref_ptr<globjects::Texture> pushBuffer;
    globjects::ref_ptr<globjects::Texture> pointBuffer;
    globjects::ref_ptr<globjects::Texture> pushPullResultBuffer;

protected:
    void render(
        const IdDrawablesMap& drawablesMap,
        const VPLProcessor& vplProcessor,
        int vplStartIndex,
        int vplEndIndex,
        bool scaleISMs,
        bool pointsOnlyIntoScaledISMs,
        float tessLevelFactor,
        bool usePushPull,
        float zFar) const;
    void pullpush(int ismPixelSize, float zFar) const;

    int m_blurSize;

    globjects::ref_ptr<globjects::Framebuffer> m_fbo;

    globjects::ref_ptr<globjects::Program> m_shadowmapProgram;
    globjects::ref_ptr<globjects::Program> m_pullProgram;
    globjects::ref_ptr<globjects::Program> m_pullLevelZeroProgram;
    globjects::ref_ptr<globjects::Program> m_pushProgram;
    globjects::ref_ptr<globjects::Program> m_pushLevelZeroProgram;
    globjects::ref_ptr<globjects::Program> m_pointSoftRenderProgram;
    globjects::ref_ptr<globjects::Buffer> m_atomicCounter;
    globjects::ref_ptr<globjects::Texture> m_atomicCounterTexture;
};
