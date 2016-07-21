#version 420

#extension GL_ARB_shading_language_include : require
#include </data/shaders/common/floatpacking.glsl>

layout(location = 0) in vec3 a_vertex;
layout(location = 1) in vec3 a_normal;

out uint v_normal;

void main()
{
    vec4 vertex = vec4(a_vertex, 1.0);

    v_normal = Pack4PNToUint(vec4(a_normal * 0.5 + 0.5, 0.0));

    gl_Position = vertex;
}
