#include "GroundPlane.h"

#include <glm/gtc/type_ptr.hpp>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>

#include <globjects/Shader.h>
#include <globjects/Program.h>

#include <gloperate/primitives/VertexDrawable.h>

using namespace gl;


namespace
{
    const auto size = 100.0f;
    const auto filePath = std::string{ "data/shaders/groundplane/" };
}

GroundPlane::GroundPlane(float height)
{
    m_program = new globjects::Program();
    m_program->attach(
        globjects::Shader::fromFile(GL_VERTEX_SHADER, filePath + "groundplane.vert"),
        globjects::Shader::fromFile(GL_FRAGMENT_SHADER, filePath + "groundplane.frag"));

    m_drawable = new gloperate::VertexDrawable(
        std::vector<glm::vec3>{ glm::vec3(-size, height, -size), glm::vec3(-size, height, size), glm::vec3(size, height, -size), glm::vec3(size, height, size) },
        gl::GL_TRIANGLE_STRIP);

    m_drawable->setFormats({ gloperate::Format(3, gl::GL_FLOAT, 0) });
    m_drawable->bindAttributes({ 0 });
    m_drawable->enableAll();
}

void GroundPlane::draw(globjects::Program* program) const
{
    if (program == nullptr) 
    {
        program = m_program;
    }

    program->use();
    m_drawable->draw();
    program->release();
}

globjects::Program * GroundPlane::program()
{
    return m_program;
}

gloperate::VertexDrawable * GroundPlane::drawable()
{
    return m_drawable;
}
