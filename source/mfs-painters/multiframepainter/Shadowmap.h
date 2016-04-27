#pragma once

#include <glm/glm.hpp>

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


class Shadowmap
{
public:
    Shadowmap();
    ~Shadowmap();

    glm::mat4 render(const glm::vec3 &eye, const glm::vec3 &direction, const IdDrawablesMap& drawablesMap, const glm::vec2& nearFar) const;
    void setBlurSize(int blurSize);

    globjects::Program * program() const;
    globjects::Texture * distanceTexture() const;

protected:
    void setupSimpleFbo(globjects::Framebuffer& fbo, globjects::Texture& VSMBuffer, int size);
    void setupFbo(globjects::Framebuffer& fbo, globjects::Texture& VSMBuffer, globjects::Texture& depthBuffer, globjects::Texture& fluxBuffer, globjects::Texture& normalBuffer, int size);

protected:
    int m_blurSize;

    globjects::ref_ptr<globjects::Framebuffer> m_fbo;
    globjects::ref_ptr<globjects::Framebuffer> m_fboFaces;
    globjects::ref_ptr<globjects::Texture> m_VSMBuffer;
    globjects::ref_ptr<globjects::Texture> m_fluxBuffer;
    globjects::ref_ptr<globjects::Texture> m_normalTexture;
    globjects::ref_ptr<globjects::Texture> m_depthBuffer;

    globjects::ref_ptr<globjects::Framebuffer> m_blurredFboTemp;
    globjects::ref_ptr<globjects::Texture> m_colorTextureBlurTemp;

    globjects::ref_ptr<globjects::Framebuffer> m_blurredFbo;
    globjects::ref_ptr<globjects::Texture> m_colorTextureBlur;

    globjects::ref_ptr<globjects::Program> m_shadowmapProgram;
	globjects::ref_ptr<globjects::Program> m_blurProgram;
};
