#version 330
#extension GL_ARB_shading_language_include : require

layout(location = 0) in vec3 a_vertex;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec3 a_uv;

out vec3 v_normal;
out vec3 v_worldCoord;
out vec3 v_uv;
out vec4 v_s;

uniform mat4 model;
uniform mat4 viewProjection;
uniform vec2 ndcOffset;
uniform vec2 cocPoint;
uniform float focalDist;

uniform mat4 biasedShadowTransform;


void main()
{
    vec4 vertex = vec4(a_vertex, 1.0);

    v_worldCoord = a_vertex;
    v_normal = a_normal;
    v_uv = a_uv;
    gl_Position = viewProjection * model * vertex;

    vec4 v_s_tmp = biasedShadowTransform * vertex;
    v_s = v_s_tmp;
}
