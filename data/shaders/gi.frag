#version 430

#extension GL_ARB_shading_language_include : require
#include </data/shaders/common/reprojection.glsl>

in vec2 v_uv;
in vec3 v_viewRay;

struct VPL {
    vec4 position;
    vec4 normal;
    vec4 color;
    mat4 viewMatrix;
};

layout (std140, binding = 0) uniform vplBuffer_
{
    VPL vplBuffer[256];
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


void main()
{
    float d = linearDepth(depthSampler, v_uv, projectionMatrix);
    vec3 fragNormal = texture(faceNormalSampler, v_uv).xyz * 2.0 - 1.0;

    vec3 fragViewCoord = d * v_viewRay;
    vec3 fragWorldCoord = (viewInvertedMatrix * vec4(fragViewCoord, 1.0)).xyz;

    vec3 acc = vec3(0.0);

    const ivec2 ismCount = ivec2(16,16);
    ivec2 from = ivec2(0, 0);
    ivec2 to = ivec2(16, 16);
    int numIterations = (to.x - from.x) * (to.y - from.y);

    for (int i = 0; i < vplBuffer.length(); i++) {
        VPL vpl = vplBuffer[i];

        vec3 vplWorldcoords = vpl.position.xyz;
        vec3 vplNormal = vpl.normal.xyz;
        vec3 vplColor = vpl.color.xyz;

        vec3 diff = fragWorldCoord - vplWorldcoords.xyz ;
        float dist = length(diff);
        vec3 normalizedDiff = diff / dist;

        // debug splotch
        float isNearLight = 1.0 - step(15.0, dist);
        vec3 debugSplotch = isNearLight * vplColor / dist / dist * 30.0;
        acc += debugSplotch;

        // angle and attenuation
        float angleFactor = max(0.0, dot(vplNormal, normalizedDiff)) * max(0.0, dot(fragNormal, -normalizedDiff));
        if (angleFactor <= 0.0)
            continue;

        float toMetersFactor = 0.01;
        float attenuation = 1.0 / pow(dist * toMetersFactor, 4.0);

        vec4 v = vpl.viewMatrix * vec4(fragWorldCoord, 1.0);

        // paraboloid projection
        v.xyz /= dist;
        v.z = 1.0 - v.z;
        v.xy /= v.z;
        v.z = dist / (2000.0);

        // scale and bias to texcoords
        v.xy += 1.0;
        v.xy /= 2.0;
        ivec2 ismIndex = ivec2(i % ismCount.x, i / ismCount.x);
        v.xy += vec2(ismIndex);
        v.xy /= ismCount;

        // ISM shadowing
        float occluderDepth = texture(ismDepthSampler, v.xy).x;
        float shadowValue = v.z - occluderDepth;
        shadowValue = smoothstep(0.90, 1.0, 1 - shadowValue);

        float arbitraryIntensityFactor = 10000.0;
        float arbitraryClampingValue = arbitraryIntensityFactor * 0.001;
        float factor =  angleFactor * attenuation * shadowValue * arbitraryIntensityFactor;
        factor = min(factor, arbitraryClampingValue);

        acc += vplColor * factor / numIterations;
    }

    outColor = vec3(acc);
}
