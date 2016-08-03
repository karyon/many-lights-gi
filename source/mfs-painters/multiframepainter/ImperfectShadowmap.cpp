#include "ImperfectShadowmap.h"

#include <limits>
#include <memory>
#include <iostream>

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
#include "PerfCounter.h"

using namespace gl;

namespace
{
    const int size = 2048;
}

ImperfectShadowmap::ImperfectShadowmap()
{
    m_shadowmapProgram = new globjects::Program();
    m_shadowmapProgram->attach(
        globjects::Shader::fromFile(GL_VERTEX_SHADER, "data/shaders/ism/ism.vert"),
        globjects::Shader::fromFile(GL_TESS_CONTROL_SHADER, "data/shaders/ism/ism.tesc"),
        globjects::Shader::fromFile(GL_TESS_EVALUATION_SHADER, "data/shaders/ism/ism.tese"),
        globjects::Shader::fromFile(GL_GEOMETRY_SHADER, "data/shaders/ism/ism.geom"),
        globjects::Shader::fromFile(GL_FRAGMENT_SHADER, "data/shaders/ism/ism.frag")
    );

    m_pullLevelZeroProgram = new globjects::Program();
    m_pullLevelZeroProgram->attach(globjects::Shader::fromFile(GL_COMPUTE_SHADER, "data/shaders/ism/pull.comp"));

    globjects::Shader::globalReplace("#define LEVEL_ZERO", "#undef LEVEL_ZERO");
    m_pullProgram = new globjects::Program();
    m_pullProgram->attach(globjects::Shader::fromFile(GL_COMPUTE_SHADER, "data/shaders/ism/pull.comp"));
    globjects::Shader::clearGlobalReplacements();

    globjects::Shader::globalReplace("#define LEVEL_ZERO", "#undef LEVEL_ZERO");
    m_pushProgram = new globjects::Program();
    m_pushProgram->attach(globjects::Shader::fromFile(GL_COMPUTE_SHADER, "data/shaders/ism/push.comp"));
    globjects::Shader::clearGlobalReplacements();

    m_pushLevelZeroProgram = new globjects::Program();
    m_pushLevelZeroProgram->attach(globjects::Shader::fromFile(GL_COMPUTE_SHADER, "data/shaders/ism/push.comp"));

    m_pointSoftRenderProgram = new globjects::Program();
    m_pointSoftRenderProgram->attach(globjects::Shader::fromFile(GL_COMPUTE_SHADER, "data/shaders/ism/ism.comp"));

    m_fbo = new globjects::Framebuffer();
    depthBuffer = globjects::Texture::createDefault();
    depthBuffer->setParameter(gl::GL_TEXTURE_MIN_FILTER, gl::GL_NEAREST);
    depthBuffer->setParameter(gl::GL_TEXTURE_MAG_FILTER, gl::GL_NEAREST);
    depthBuffer->setName("ISM Depth");
    depthBuffer->storage2D(1, GL_DEPTH_COMPONENT16, size, size);
    m_fbo->attachTexture(GL_DEPTH_ATTACHMENT, depthBuffer);

    softrenderBuffer = globjects::Texture::createDefault(GL_TEXTURE_3D);
    softrenderBuffer->setParameter(gl::GL_TEXTURE_MIN_FILTER, gl::GL_NEAREST);
    softrenderBuffer->setParameter(gl::GL_TEXTURE_MAG_FILTER, gl::GL_NEAREST);
    softrenderBuffer->setName("ISM softrender");
    softrenderBuffer->storage3D(1, GL_R32UI, size, size, 2);

    m_fbo->printStatus(true);


    pullBuffer = new globjects::Texture(GL_TEXTURE_2D);
    pullBuffer->setName("Pull Buffer");
    pullBuffer->storage2D(10, GL_RGBA32F, size, size);

    pushBuffer = new globjects::Texture(GL_TEXTURE_2D);
    pushBuffer->setName("Push Buffer");
    pushBuffer->storage2D(10, GL_RGBA32F, size, size);
    pushBuffer->setParameter(gl::GL_TEXTURE_MIN_FILTER, gl::GL_NEAREST_MIPMAP_NEAREST);
    pushBuffer->setParameter(gl::GL_TEXTURE_MAG_FILTER, gl::GL_NEAREST);

    auto b = new globjects::Buffer();
    b->setData(sizeof(glm::vec4) * (1 << 22) , nullptr, GL_STATIC_DRAW);
    pointBuffer = new globjects::Texture(GL_TEXTURE_BUFFER);
    pointBuffer->setName("Point Buffer");
    pointBuffer->texBuffer(GL_RGBA32F, b);


    pushPullResultBuffer = new globjects::Texture(GL_TEXTURE_2D);
    pushPullResultBuffer->setName("Pushpull result");
    pushPullResultBuffer->storage2D(1, GL_R16, size, size);
    pushPullResultBuffer->setParameter(gl::GL_TEXTURE_MIN_FILTER, gl::GL_NEAREST);
    pushPullResultBuffer->setParameter(gl::GL_TEXTURE_MAG_FILTER, gl::GL_NEAREST);

    m_atomicCounter = new globjects::Buffer();
    m_atomicCounter->setName("atomic counter");
    m_atomicCounter->setData(sizeof(gl::GLuint)*1024 * 4, nullptr, GL_STATIC_DRAW);
    m_atomicCounterTexture = new globjects::Texture(GL_TEXTURE_BUFFER);
    m_atomicCounterTexture->setName("pointCounterTexture");
    //m_atomicCounterTexture->texBuffer(GL_R32UI, b);
}

ImperfectShadowmap::~ImperfectShadowmap()
{

}

void ImperfectShadowmap::pull() const
{
    AutoGLDebugGroup c("ISM pushpull");

    softrenderBuffer->bindActive(0);

    // TODO investigate why 2nd level is only 2 times faster
    // TODO maybe merge this and let one dispatch do multiple levels
    for (int i = 0; i <= 5; i++) {
        if (i <= 2)
            PerfCounter::beginGL("PL" + std::to_string(i));

        gl::glMemoryBarrier(gl::GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        pullBuffer->bindImageTexture(0, i, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        pullBuffer->bindImageTexture(1, i + 1, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        auto program = (i == 0) ? m_pullLevelZeroProgram : m_pullProgram;
        program->setUniform("level", i);
        program->dispatchCompute(size / int(std::pow(2, i + 1)) / 8, size / int(std::pow(2, i + 1)) / 8, 1);

        if (i <= 2)
            PerfCounter::endGL("PL" + std::to_string(i));
    }

    pushPullResultBuffer->bindImageTexture(3, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16);

    for (int i = 5; i >= 0; i--) {
        if (i <= 2)
            PerfCounter::beginGL("PS" + std::to_string(i));

        gl::glMemoryBarrier(gl::GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        pullBuffer->bindImageTexture(0, i, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        // in the first step, read directly from pullBuffer as there is no pushBuffer yet
        auto readTexture = (i == 5) ? pullBuffer : pushBuffer;
        readTexture->bindImageTexture(1, i+1, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        pushBuffer->bindImageTexture(2, i, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        auto currResultBuffer = (i == 0) ? pushPullResultBuffer : pushBuffer;
        auto program = (i == 0) ? m_pushLevelZeroProgram : m_pushProgram;
        program->setUniform("level", i);
        program->dispatchCompute(size / int(std::pow(2, i)) / 8, size / int(std::pow(2, i)) / 8, 1);

        if (i <= 2)
            PerfCounter::endGL("PS" + std::to_string(i));
    }
}

void ImperfectShadowmap::process(const IdDrawablesMap& drawablesMap, const VPLProcessor& vplProcessor, int vplStartIndex, int vplEndIndex, bool scaleISMs, bool pointsOnlyIntoScaledISMs, float tessLevelFactor, bool usePushPull, float zFar) const
{
    render(drawablesMap, vplProcessor, vplStartIndex, vplEndIndex, scaleISMs, pointsOnlyIntoScaledISMs, tessLevelFactor, usePushPull, zFar);
    pull();
}

void ImperfectShadowmap::render(const IdDrawablesMap& drawablesMap, const VPLProcessor& vplProcessor, int vplStartIndex, int vplEndIndex, bool scaleISMs, bool pointsOnlyIntoScaledISMs, float tessLevelFactor, bool usePushPull, float zFar) const
{
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glViewport(0, 0, size, size);

    m_fbo->bind();

    m_fbo->clearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);

    m_fbo->clearBuffer(GL_COLOR, 0, glm::vec4(0.0f));

    vplProcessor.packedVplBuffer->bindBase(GL_UNIFORM_BUFFER, 0);
    gl::GLuint zero = 0;
    m_atomicCounter->clearData(GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);
    m_atomicCounter->bindBase(GL_SHADER_STORAGE_BUFFER, 0);

    softrenderBuffer->clearImage(0, GL_RED_INTEGER, GL_UNSIGNED_INT, glm::uvec4(500000));
    softrenderBuffer->bindImageTexture(0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    pointBuffer->bindImageTexture(1, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    m_shadowmapProgram->setUniform("viewport", glm::ivec2(size, size));
    m_shadowmapProgram->setUniform("zFar", zFar);
    m_shadowmapProgram->setUniform("vplStartIndex", vplStartIndex);
    m_shadowmapProgram->setUniform("vplEndIndex", vplEndIndex);
    m_shadowmapProgram->setUniform("scaleISMs", scaleISMs);
    m_shadowmapProgram->setUniform("pointsOnlyIntoScaledISMs", pointsOnlyIntoScaledISMs);
    m_shadowmapProgram->setUniform("usePushPull", usePushPull);
    m_shadowmapProgram->setUniform("tessLevelFactor", tessLevelFactor);

    m_shadowmapProgram->use();

    glEnable(GL_PROGRAM_POINT_SIZE);
    glPatchParameteri(GL_PATCH_VERTICES, 3);

    AutoGLPerfCounter c("ISM render");
    for (const auto& pair : drawablesMap)
    {
        auto& drawables = pair.second;
        for (auto& drawable : drawables)
        {
            drawable->draw(GL_PATCHES);
        }
    }
    m_shadowmapProgram->release();

    if (usePushPull) {
        gl::glMemoryBarrier(gl::GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        gl::glMemoryBarrier(gl::GL_SHADER_STORAGE_BARRIER_BIT);

        m_pointSoftRenderProgram->setUniform("viewport", glm::ivec2(size, size));
        m_pointSoftRenderProgram->setUniform("zFar", zFar);
        m_pointSoftRenderProgram->setUniform("vplStartIndex", vplStartIndex);
        m_pointSoftRenderProgram->setUniform("vplEndIndex", vplEndIndex);
        m_pointSoftRenderProgram->setUniform("scaleISMs", scaleISMs);
        m_pointSoftRenderProgram->setUniform("pointsOnlyIntoScaledISMs", pointsOnlyIntoScaledISMs);
        m_pointSoftRenderProgram->setUniform("usePushPull", usePushPull);
        m_pointSoftRenderProgram->setUniform("tessLevelFactor", tessLevelFactor);
        m_pointSoftRenderProgram->dispatchCompute(1024, 1, 1);

    }


}
