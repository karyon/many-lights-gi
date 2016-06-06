#version 430

#extension GL_ARB_shading_language_include : require
#include </data/shaders/common/reprojection.glsl>
#include </data/shaders/common/ism_utils.glsl>

in vec2 v_uv;
in vec3 v_viewRay;

struct VPL {
    vec4 position;
    vec4 normal;
    vec4 color;
};

const int totalVplCount = 256;
layout (std140, binding = 0) uniform vplBuffer_
{
    VPL vplBuffer[totalVplCount];
};

out vec3 outColor;

uniform sampler2D faceNormalSampler;
uniform sampler2D depthSampler;
uniform sampler2D ismDepthSampler;

uniform mat4 projectionMatrix;
uniform mat4 projectionInverseMatrix;
uniform mat4 viewMatrix;
uniform mat4 viewInvertedMatrix;
uniform float zFar;
uniform float zNear;

uniform float giIntensityFactor;
uniform float vplClampingValue;

// don't forget that these uniforms eat performance
uniform int vplStartIndex = 0;
uniform int vplEndIndex = 256;
int vplCount = vplEndIndex - vplStartIndex;
uniform bool scaleISMs = false;
float ismIndexOffset = scaleISMs ? vplStartIndex : 0;
int ismCount = (scaleISMs) ? vplCount : totalVplCount;
int ismIndices1d = int(ceil(sqrt(ismCount)));

uniform bool showLightPositions = false;

void main()
{
    float d = linearDepth(depthSampler, v_uv, projectionMatrix);
    vec3 fragNormal = texture(faceNormalSampler, v_uv).xyz * 2.0 - 1.0;

    vec3 fragViewCoord = d * v_viewRay;
    vec3 fragWorldCoord = (viewInvertedMatrix * vec4(fragViewCoord, 1.0)).xyz;

    vec3 acc = vec3(0.0);

    for (int vplIndex = vplStartIndex; vplIndex < vplEndIndex; vplIndex++) {
        VPL vpl = vplBuffer[vplIndex];

        vec3 vplWorldcoords = vpl.position.xyz;
        vec3 vplNormal = vpl.normal.xyz;
        vec3 vplColor = vpl.color.xyz;

        vec3 diff = fragWorldCoord - vplWorldcoords.xyz ;
        float dist = length(diff);
        vec3 normalizedDiff = diff / dist;

        // debug splotch
        float isNearLight = 1.0 - step(0.15, dist);
        vec3 debugSplotch = isNearLight * vplColor / dist / dist * 0.0001;
        acc += debugSplotch * float(showLightPositions);

        // angle and attenuation
        float angleFactor = max(0.0, dot(vplNormal, normalizedDiff)) * max(0.0, dot(fragNormal, -normalizedDiff));
        if (angleFactor <= 0.0)
            continue;

        float attenuation = 1.0 / pow(dist, 4.0);

        // paraboloid projection
        float ismIndex = vplIndex - ismIndexOffset;
        vec3 v = paraboloid_project(diff, dist, vplNormal, zFar, ismIndex, ismIndices1d);

        // ISM shadowing
        float occluderDepth = texture(ismDepthSampler, v.xy).x;
        float shadowValue = v.z - occluderDepth;
        shadowValue = smoothstep(0.95, 1.0, 1 - shadowValue);

        float geometryTerm = angleFactor * attenuation;
        geometryTerm = min(geometryTerm, vplClampingValue);

        acc += vplColor * geometryTerm * shadowValue;
    }
    outColor = vec3(acc * giIntensityFactor / vplCount);
}
