#version 330
#extension GL_ARB_shading_language_include : require

#include </data/shaders/common/transformations.glsl>

layout(location = 0) in vec3 in_position;

uniform mat4 modelView;
uniform mat4 projection;
uniform vec2 ndcOffset;
uniform vec2 cocPoint;
uniform float focalDist;

out vec3 g_worldCoord;

void main()
{
    vec4 viewVertex = depthOfField(modelView, vec4(in_position, 1.0), cocPoint, focalDist);
    vec4 ndcVertex = subpixelShift(projection, viewVertex, ndcOffset);

    gl_Position = ndcVertex;
    g_worldCoord = in_position;
}
