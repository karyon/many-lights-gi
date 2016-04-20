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
out vec3 v_s;

uniform mat4 biasedShadowTransform;

void main()
{
    vec4 vertex = vec4(in_position, 1.0);
    vec4 viewVertex = depthOfField(modelView, vertex, cocPoint, focalDist);
    vec4 ndcVertex = subpixelShift(projection, viewVertex, ndcOffset);

    gl_Position = ndcVertex;
    g_worldCoord = in_position;

    vec4 v_s_tmp = biasedShadowTransform * vertex;
    v_s = v_s_tmp.xyz / v_s_tmp.w;
}
