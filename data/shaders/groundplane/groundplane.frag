#version 330
#extension GL_ARB_shading_language_include : require

#include </data/shaders/common/shadowmapping.glsl>

uniform sampler2D shadowmap;
uniform vec3 worldLightPos;
uniform vec3 groundPlaneColor;

in vec3 g_worldCoord;
in vec3 v_s;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outWorldPos;
layout (location = 3) out float reflects;

const float ambientFactor = 0.25;

void main()
{
    outColor = groundPlaneColor * shadowmapComparisonVSM(shadowmap, v_s.xy, g_worldCoord, worldLightPos);
    outColor += ambientFactor * groundPlaneColor;
    outColor = clamp(outColor, 0.0, 1.0);

    outNormal = vec3(0.0, 1.0, 0.0);
    outWorldPos = g_worldCoord;
    reflects = 1.0;
}
