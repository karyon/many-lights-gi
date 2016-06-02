#include "ImperfectShadowmap.h"

#include <limits>
#include <memory>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>
#include <glbinding/gl/bitfield.h>

#include <globjects/Shader.h>
#include <globjects/Program.h>
#include <globjects/Texture.h>
#include <globjects/Framebuffer.h>
#include <globjects/Buffer.h>

#include <gloperate/primitives/VertexDrawable.h>
#include <gloperate/primitives/PolygonalDrawable.h>
#include <gloperate/painter/AbstractProjectionCapability.h>
#include <gloperate/painter/AbstractCameraCapability.h>

#include "VPLProcessor.h"

using namespace gl;

namespace
{
    const int size = 2048;
}

ImperfectShadowmap::ImperfectShadowmap()
{
    m_shadowmapProgram = new globjects::Program();
    m_shadowmapProgram->attach(
        globjects::Shader::fromFile(GL_VERTEX_SHADER, "data/shaders/ism.vert"),
        globjects::Shader::fromFile(GL_TESS_CONTROL_SHADER, "data/shaders/ism.tesc"),
        globjects::Shader::fromFile(GL_TESS_EVALUATION_SHADER, "data/shaders/ism.tese"),
        globjects::Shader::fromFile(GL_GEOMETRY_SHADER, "data/shaders/ism.geom"),
        globjects::Shader::fromFile(GL_FRAGMENT_SHADER, "data/shaders/empty.frag")
    );

    m_blurProgram = new globjects::Program();
    m_blurProgram->attach(
        globjects::Shader::fromFile(GL_VERTEX_SHADER, "data/shaders/cubemapblur.vert"),
        globjects::Shader::fromFile(GL_GEOMETRY_SHADER, "data/shaders/cubemapblur.geom"),
        globjects::Shader::fromFile(GL_FRAGMENT_SHADER, "data/shaders/cubemapblur.frag")
    );

    m_fbo = new globjects::Framebuffer();
    depthBuffer = globjects::Texture::createDefault();
    depthBuffer->setName("ISM Depth");

    depthBuffer->image2D(0, GL_DEPTH_COMPONENT, size, size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    m_fbo->attachTexture(GL_DEPTH_ATTACHMENT, depthBuffer);

    m_fbo->printStatus(true);

    //m_blurredFbo = new globjects::Framebuffer();
    //m_colorTextureBlur = globjects::Texture::createDefault();
    //setupSimpleFbo(*m_blurredFbo, *m_colorTextureBlur, size);

    //m_blurredFboTemp = new globjects::Framebuffer();
    //m_colorTextureBlurTemp = globjects::Texture::createDefault();
    //setupSimpleFbo(*m_blurredFboTemp, *m_colorTextureBlurTemp, size);
}

ImperfectShadowmap::~ImperfectShadowmap()
{

}

void ImperfectShadowmap::setupFbo(globjects::Framebuffer& fbo, globjects::Texture& depthBuffer, int size)
{

}


void ImperfectShadowmap::render(const IdDrawablesMap& drawablesMap, const VPLProcessor& vplProcessor, int vplStartIndex, int vplEndIndex, bool scaleISMs, bool pointsOnlyIntoScaledISMs, float tessLevelFactor) const
{
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glViewport(0, 0, size, size);

    m_fbo->bind();

    m_fbo->clearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);

    vplProcessor.vplBuffer->bindBase(GL_UNIFORM_BUFFER, 0);

    m_shadowmapProgram->setUniform("viewport", glm::ivec2(size, size));
    m_shadowmapProgram->setUniform("vplStartIndex", vplStartIndex);
    m_shadowmapProgram->setUniform("vplEndIndex", vplEndIndex);
    m_shadowmapProgram->setUniform("scaleISMs", scaleISMs);
    m_shadowmapProgram->setUniform("pointsOnlyIntoScaledISMs", pointsOnlyIntoScaledISMs);
    m_shadowmapProgram->setUniform("tessLevelFactor", tessLevelFactor);

    m_shadowmapProgram->use();

    glEnable(GL_PROGRAM_POINT_SIZE);
    glPatchParameteri(GL_PATCH_VERTICES, 3);

    for (const auto& pair : drawablesMap)
    {
        auto& drawables = pair.second;
        for (auto& drawable : drawables)
        {
            drawable->draw(GL_PATCHES);
        }
    }

    m_shadowmapProgram->release();

    //m_blurProgram->setUniform("transform", projection*view);
    //m_blurProgram->setUniform("sizeFactor", 1.0f / size);
    //m_blurProgram->setUniform("kernelsize", m_blurSize);
    //m_blurProgram->setUniform("shadowMap", 0);

    //// blur x pass
    //m_blurProgram->use();

    //m_blurredFboTemp->bind();
    //m_colorTexture->bindActive(GL_TEXTURE0);
    //m_blurProgram->setUniform("direction", glm::vec2(1.0f, 0.0f));

    //m_cube->draw();

    //// blur y pass
    //m_blurredFbo->bind();
    //m_colorTextureBlurTemp->bindActive(GL_TEXTURE0);
    //m_blurProgram->setUniform("direction", glm::vec2(0.0f, 1.0f));

    //m_cube->draw();

    //m_colorTextureBlurTemp->unbindActive(GL_TEXTURE0);
    //m_blurProgram->release();
    //m_blurredFbo->unbind();
}
