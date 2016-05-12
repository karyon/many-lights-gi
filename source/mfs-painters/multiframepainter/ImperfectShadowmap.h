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


class ImperfectShadowmap
{
public:
    ImperfectShadowmap();
    ~ImperfectShadowmap();

    void render(const glm::vec3 &eye, const glm::mat4 &view, const IdDrawablesMap& drawablesMap, const glm::vec2& nearFar) const;

    globjects::ref_ptr<globjects::Texture> depthBuffer;

protected:
    void setupFbo(globjects::Framebuffer& fbo, globjects::Texture& depthBuffer, int size);

protected:
    int m_blurSize;

    globjects::ref_ptr<globjects::Framebuffer> m_fbo;
    globjects::ref_ptr<globjects::Framebuffer> m_fboFaces;

    globjects::ref_ptr<globjects::Framebuffer> m_blurredFboTemp;
    globjects::ref_ptr<globjects::Texture> m_colorTextureBlurTemp;

    globjects::ref_ptr<globjects::Framebuffer> m_blurredFbo;
    globjects::ref_ptr<globjects::Texture> m_colorTextureBlur;

    globjects::ref_ptr<globjects::Program> m_shadowmapProgram;
	globjects::ref_ptr<globjects::Program> m_blurProgram;
};
