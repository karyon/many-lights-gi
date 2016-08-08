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


class Shadowmap
{
public:
    Shadowmap();
    ~Shadowmap();

    void render(const glm::vec3 &eye, const glm::mat4 &viewProjection, const IdDrawablesMap& drawablesMap, const IdMaterialMap& materialMap, const glm::vec2& nearFar) const;

    globjects::Program * program() const;

    globjects::ref_ptr<globjects::Texture> vsmBuffer;
    globjects::ref_ptr<globjects::Texture> depthBuffer;

protected:
    void setupSimpleFbo(globjects::Framebuffer& fbo, globjects::Texture& VSMBuffer, int size);
    void setupFbo(globjects::Framebuffer& fbo, globjects::Texture& VSMBuffer, globjects::Texture& depthBuffer, int size);

protected:
    int m_blurSize;

    globjects::ref_ptr<globjects::Framebuffer> m_fbo;
    globjects::ref_ptr<globjects::Framebuffer> m_fboFaces;

    globjects::ref_ptr<globjects::Program> m_shadowmapProgram;
};
