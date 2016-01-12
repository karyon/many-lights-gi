#version 330
#extension GL_ARB_shading_language_include : require

#include </data/shaders/common/transformations.glsl>

layout(location = 0) in vec3 a_vertex;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec3 a_uv;

out vec3 v_normal;
out vec3 v_worldCoord;
out vec3 v_uv;

uniform mat4 modelView;
uniform mat4 projection;
uniform vec2 ndcOffset;
uniform vec2 cocPoint;
uniform float focalDist;

void main()
{
    vec4 viewVertex = depthOfField(modelView, vec4(a_vertex, 1.0), cocPoint, focalDist);
    vec4 ndcVertex = subpixelShift(projection, viewVertex, ndcOffset);

    v_worldCoord = a_vertex;
    v_normal = a_normal;
    v_uv = a_uv;
    gl_Position = ndcVertex;
}
