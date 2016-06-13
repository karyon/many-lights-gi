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

float weight(float r, float depthDiff)
{
    const float BlurSigma = 1.4; // unsure why i had to fine-tune that
    const float BlurFalloff = 1.0 / (2.0*BlurSigma*BlurSigma);
    float w = exp2(-r*r*BlurFalloff - depthDiff*depthDiff);
    return w;
}

void main()
{
    float d = linearDepth(depthSampler, v_uv, projectionMatrix);

    vec3 N = texture(faceNormalSampler, v_uv, 0).xyz * 2.0 - 1.0;

    const ivec2 offsets[NUM_SAMPLES] = {
        // DIRECTION * -4,
        DIRECTION * -3,
        DIRECTION * -2,
        DIRECTION * -1,
        DIRECTION * 0,
        DIRECTION * 1,
        DIRECTION * 2,
        DIRECTION * 3,
        // DIRECTION * 4,
    };
    const ivec2 weights[NUM_SAMPLES] = {
        // DIRECTION * -4,
        DIRECTION * -3,
        DIRECTION * -2,
        DIRECTION * -1,
        DIRECTION * 0,
        DIRECTION * 1,
        DIRECTION * 2,
        DIRECTION * 3,
        // DIRECTION * 4,
    };


    vec3 acc = vec3(0.0);
    float factorAcc = 0.0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        ivec2 texcoord = ivec2(gl_FragCoord.xy) + offsets[i];

        vec3 giSample = texelFetch(giSampler, texcoord, 0).xyz;
        vec3 normalSample = texelFetch(faceNormalSampler, texcoord, 0).xyz * 2.0 - 1.0;
        float depthSample = linearDepth(depthSampler, texcoord, projectionMatrix);

        float normalFactor = max(0, dot((N), normalSample));
        float depthDiff = (depthSample - d) / 1;
        float factor =  weight(i - KERNEL_RADIUS, depthDiff) * normalFactor;
        factorAcc += factor;
        acc += giSample * factor;
    }

    outColor = acc / factorAcc;
}
