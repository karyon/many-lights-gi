#pragma once

#include <globjects/base/ref_ptr.h>

namespace globjects
{
    class Program;
    class Texture;
}

namespace gloperate
{
    class VertexDrawable;
}


class GroundPlane
{
public:
    GroundPlane(float height);

    void draw(globjects::Program* program = nullptr) const;
    globjects::Program * program();
    gloperate::VertexDrawable * drawable();

protected:
    globjects::ref_ptr<globjects::Program> m_program;
    globjects::ref_ptr<gloperate::VertexDrawable> m_drawable;
};
