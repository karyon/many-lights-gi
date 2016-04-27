#version 140
#extension GL_ARB_explicit_attrib_location : require

uniform mat4 projectionInverseMatrix;

layout (location = 0) in vec2 a_vertex;

out vec2 v_uv;
out vec3 v_viewRay;

void main()
{
    v_uv = a_vertex * 0.5 + 0.5;
    gl_Position = vec4(a_vertex, 0.0, 1.0);

    vec4 vertex_vs = projectionInverseMatrix * vec4(a_vertex, 1.0, 1.0);
    v_viewRay = vertex_vs.xyz / vertex_vs.w;
}
