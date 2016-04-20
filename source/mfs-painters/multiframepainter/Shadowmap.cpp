#include "Shadowmap.h"

#include <limits>
#include <memory>

#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>
#include <glbinding/gl/bitfield.h>

#include <globjects/Shader.h>
#include <globjects/Program.h>
#include <globjects/Texture.h>
#include <globjects/Framebuffer.h>

#include <gloperate/primitives/VertexDrawable.h>
#include <gloperate/primitives/PolygonalDrawable.h>

#include "GroundPlane.h"

using namespace gl;

namespace
{
    const int size = 512;
}

Shadowmap::Shadowmap()
{
    m_shadowmapProgram = new globjects::Program();
    m_shadowmapProgram->attach(
        globjects::Shader::fromFile(GL_VERTEX_SHADER, "data/shaders/shadowmapping_nonomni.vert"),
        globjects::Shader::fromFile(GL_FRAGMENT_SHADER, "data/shaders/shadowmapping.frag"));

    m_blurProgram = new globjects::Program();
    m_blurProgram->attach(
        globjects::Shader::fromFile(GL_VERTEX_SHADER, "data/shaders/cubemapblur.vert"),
        globjects::Shader::fromFile(GL_GEOMETRY_SHADER, "data/shaders/cubemapblur.geom"),
        globjects::Shader::fromFile(GL_FRAGMENT_SHADER, "data/shaders/cubemapblur.frag"));

    m_fbo = new globjects::Framebuffer();
    m_colorTexture = globjects::Texture::createDefault();
    m_depthTexture = globjects::Texture::createDefault();
    setupFbo(m_fbo, m_colorTexture, m_depthTexture, size);

    m_blurredFbo = new globjects::Framebuffer();
    m_colorTextureBlur = globjects::Texture::createDefault();
    setupFbo(m_blurredFbo, m_colorTextureBlur, nullptr, size);

    m_blurredFboTemp = new globjects::Framebuffer();
    m_colorTextureBlurTemp = globjects::Texture::createDefault();
    setupFbo(m_blurredFboTemp, m_colorTextureBlurTemp, nullptr, size);
}

void Shadowmap::setupFbo(globjects::Framebuffer * fbo, globjects::Texture * colorBuffer, globjects::Texture * depthBuffer, int size)
{
    colorBuffer->bind();


	colorBuffer->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	colorBuffer->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glm::vec4 color(0.0, 0.0, 1.0, 0.0); // third channel is alpha
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, (float*)&color);

	colorBuffer->image2D(0, GL_RGBA32F, size, size, 0, GL_RGB, GL_FLOAT, nullptr);
    colorBuffer->unbind();
    fbo->attachTexture(GL_COLOR_ATTACHMENT0, colorBuffer);

    if (depthBuffer)
    {
        depthBuffer->bind();
		depthBuffer->image2D(0, GL_DEPTH_COMPONENT, size, size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        depthBuffer->unbind();
        fbo->attachTexture(GL_DEPTH_ATTACHMENT, depthBuffer);
    }

    fbo->setDrawBuffer(GL_COLOR_ATTACHMENT0);
    fbo->checkStatus();
    fbo->unbind();
}

glm::mat4 Shadowmap::render(const glm::vec3 &eye, const IdDrawablesMap& drawablesMap, const GroundPlane& groundPlane, float nearPlane, float farPlane) const
{


	auto projection = glm::perspective(glm::radians(90.0f), 1.0f, nearPlane, farPlane);
	auto view = glm::lookAt(eye, eye + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));


    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glViewport(0, 0, size, size);

    m_fbo->bind();
    m_fbo->setDrawBuffer(GL_COLOR_ATTACHMENT0);
    
    auto maxFloat = std::numeric_limits<float>::max();
    m_fbo->clearBuffer(GL_COLOR, 0, glm::vec4(maxFloat, maxFloat, 1.0f, 0.0f));
    m_fbo->clearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);

	auto transform = projection * view;
    m_shadowmapProgram->setUniform("transform", transform);
    m_shadowmapProgram->setUniform("lightWorldPos", eye);

	auto shadowBias = glm::mat4(
		0.5f, 0.0f, 0.0f, 0.0f
		, 0.0f, 0.5f, 0.0f, 0.0f
		, 0.0f, 0.0f, 0.5f, 0.0f
		, 0.5f, 0.5f, 0.5f, 1.0f);

	glm::mat4 biasedShadowTransform = shadowBias * transform;

    m_shadowmapProgram->use();
    for (const auto& pair : drawablesMap)
    {
        auto& drawables = pair.second;
        for (auto& drawable : drawables)
        {
            drawable->draw();
        }
    }

    groundPlane.draw(m_shadowmapProgram);

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

	return biasedShadowTransform;
}

void Shadowmap::setBlurSize(int blurSize)
{
    m_blurSize = blurSize;
}

globjects::Program* Shadowmap::program() const
{
    return m_shadowmapProgram;
}

globjects::Texture * Shadowmap::distanceTexture() const
{
    return m_colorTexture;
}
