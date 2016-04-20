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


class GroundPlane;

class Shadowmap
{
public:
    Shadowmap();

    glm::mat4 render(const glm::vec3 &eye, const IdDrawablesMap& drawablesMap, const GroundPlane& groundPlane, float nearPlane, float farPlane) const;
    void setBlurSize(int blurSize);

    globjects::Program * program() const;
    globjects::Texture * distanceTexture() const;

protected:
    void setupFbo(globjects::Framebuffer * fbo, globjects::Texture * colorBuffer, globjects::Texture * depthBuffer, int size);

protected:
    int m_blurSize;

    globjects::ref_ptr<globjects::Framebuffer> m_fbo;
    globjects::ref_ptr<globjects::Framebuffer> m_fboFaces;
    globjects::ref_ptr<globjects::Texture> m_colorTexture;
    globjects::ref_ptr<globjects::Texture> m_depthTexture;

    globjects::ref_ptr<globjects::Framebuffer> m_blurredFboTemp;
    globjects::ref_ptr<globjects::Texture> m_colorTextureBlurTemp;

    globjects::ref_ptr<globjects::Framebuffer> m_blurredFbo;
    globjects::ref_ptr<globjects::Texture> m_colorTextureBlur;

    globjects::ref_ptr<globjects::Program> m_shadowmapProgram;
	globjects::ref_ptr<globjects::Program> m_blurProgram;
};
