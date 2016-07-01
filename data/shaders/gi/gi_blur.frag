#version 430

#extension GL_ARB_shading_language_include : require
#include </data/shaders/common/reprojection.glsl>

in vec2 v_uv;
in vec3 v_viewRay;

out vec3 outColor;

uniform sampler2D giSampler;
uniform sampler2D faceNormalSampler;
uniform sampler2D depthSampler;

uniform mat4 projectionMatrix;
uniform mat4 projectionInverseMatrix;

// global replacement
#define DIRECTION ivec2(0,0)

const int KERNEL_RADIUS = 3;
const int NUM_SAMPLES = 2*KERNEL_RADIUS+1;

float weight(float r, float depthDiff, float normalDiff)
{
    // const float BlurSigma = 1.4; // unsure why i had to fine-tune that
    // const float BlurFalloff = 1.0 / (2.0*BlurSigma*BlurSigma);
    // float w = exp2(-r*r*BlurFalloff - depthDiff*depthDiff - normalDiff*normalDiff);

    const float factor = (KERNEL_RADIUS - r + 1) / 4;
    float w = factor - depthDiff*depthDiff - normalDiff*normalDiff;
    w = max(0, w);

    return w;
}

void processSample(int i, ivec2 center, ivec2 offset, vec3 centerNormal, float centerDepth, inout vec3 acc, inout float factorAcc)
{
    ivec2 texcoord = center + offset;
    vec3 giSample = texelFetch(giSampler, texcoord, 0).xyz;
    vec3 normalSample = texelFetch(faceNormalSampler, texcoord, 0).xyz * 2.0 - 1.0;
    float depthSample = linearDepth(depthSampler, texcoord, projectionMatrix);

    float normalFactor = 1 - max(0, dot((centerNormal), normalSample));
    float depthDiff = (depthSample - centerDepth) / 1;
    float factor =  weight(i, depthDiff, normalFactor*5);
    factorAcc += factor;
    acc += giSample * factor;
}

void main()
{
    vec3 acc = vec3(0.0);
    float factorAcc = 0.0;

    // center sample
    ivec2 center = ivec2(gl_FragCoord.xy);
    float d = linearDepth(depthSampler, v_uv, projectionMatrix);
    vec3 N = texture(faceNormalSampler, v_uv, 0).xyz * 2.0 - 1.0;
    acc += texelFetch(giSampler, center, 0).xyz;
    factorAcc += 1.0;

    for (int i = 1; i <= KERNEL_RADIUS; i++) {
        processSample(i, center, -DIRECTION*i, N, d, acc, factorAcc);
    }

    for (int i = 1; i <= KERNEL_RADIUS; i++) {
        processSample(i, center, DIRECTION*i, N, d, acc, factorAcc);
    }

    outColor = acc / factorAcc;
}
