#version 330

in vec2 v_uv;

out vec3 outColor;

uniform sampler2D colorSampler;
uniform sampler2D normalSampler;
uniform sampler2D depthSampler;
uniform sampler1D ssaoKernelSampler;
uniform sampler2D ssaoNoiseSampler;
uniform sampler1D reflectionKernelSampler;
uniform sampler2D worldPosSampler;
uniform sampler2D reflectSampler;

uniform bool useReflections;
uniform float zThickness;
uniform mat3 normalMatrix;
uniform mat4 projectionMatrix;
uniform mat4 projectionInverseMatrix;
uniform mat4 view;
uniform float farZ;
uniform vec2 screenSize;
uniform vec4 samplerSizes;
uniform vec3 cameraEye;
uniform float ssaoRadius;
uniform int currentFrame;

const float maxTraceSteps = 30.0;
const float pixelPerStep = 5.0;

vec3 worldToCamera(vec3 pos)
{
    vec4 pos4 = view * vec4(pos, 1.0);
    return pos4.xyz / pos4.w;
}

vec3 cameraToNDC(vec3 pos)
{
    vec4 pos4 = projectionMatrix * vec4(pos, 1.0);
    return pos4.xyz / pos4.w;
}

vec3 NDCToCamera(vec3 pos)
{
    vec4 pos4 = projectionInverseMatrix * vec4(pos, 1.0);
    return pos4.xyz / pos4.w;
}

bool equalsDelta(float v1, float v2, float d)
{
    return v1 > v2 - d && v1 < v2 + d;
}

bool traceScreenSpaceRay(vec3 csOrig, vec3 csDir, sampler2D csBuffer, float zThickness, float pixelPerStep, const float maxSteps, const float maxDist, out vec2 hitPixel, out float strength)
{
    hitPixel = vec2(-1.0);
    vec3 ndc1 = cameraToNDC(csOrig);
    vec3 ndc2 = cameraToNDC(csOrig + csDir * maxDist);
    vec3 deltaNDC = ndc2 - ndc1;
    vec2 deltaScreen = ndc2.xy - ndc1.xy;

    // calculate size of step for initial search
    vec2 pixelStep = (1.0 / textureSize(csBuffer, 0)) * 2;
    float steps = length(deltaScreen) / length(pixelStep);
    steps /= pixelPerStep;
    float stepSize = length(deltaNDC) / steps;

    vec3 curNDC = ndc1;
    vec3 lastNDC = curNDC;
    for (float i = 1.0; i <= steps; i += 1.0)
    {
        if (i > maxSteps)
            break;

        curNDC += deltaNDC * stepSize;
        if (any(greaterThanEqual(curNDC.xy, vec2(1.0))) || any(lessThanEqual(curNDC.xy, vec2(-1.0))))
        {
            return false;
        }

        float depth = NDCToCamera(curNDC).z;
        vec2 texCoord = curNDC.xy * 0.5 + 0.5;

        vec3 worldPos = texture(csBuffer, texCoord).rgb;
        if (all(greaterThan(worldPos, vec3(100000.0))))
        {
            continue;
        }
        vec3 comp = worldToCamera(worldPos);

        if (depth < comp.z - zThickness)
        {
            continue;
        }

        // hit was detected, try binary search to find exact reflection point
        if (depth < comp.z)
        {
            strength = 1.0 / i;
            vec3 delta = lastNDC - curNDC;
            float minDist = 0.0;
            float maxDist = 1.0;
            int count = 0;

            while (minDist < maxDist)
            {
                if (count++ > 10)
                    break;

                float curDist = minDist + 0.5 * (maxDist - minDist);
                vec3 curPoint = curNDC + curDist * delta;

                float depth = NDCToCamera(curPoint).z;
                vec2 texCoord = curPoint.xy * 0.5 + 0.5;

                vec3 worldPos = texture(csBuffer, texCoord).rgb;
                if (all(greaterThan(worldPos, vec3(100000.0))))
                {
                    continue;
                }
                vec3 comp = worldToCamera(worldPos);

                if (equalsDelta(depth, comp.z, maxDist / 50.0))
                {
                    hitPixel = texCoord;
                    return true;
                }

                if (depth < comp.z)
                {
                    minDist = curDist;
                }
                else
                {
                    maxDist = curDist;
                }
            }

            hitPixel = texCoord;
            return true;
        }

        lastNDC = curNDC;
    }
    return false;
}

// returns values in [nearZ:farZ]
float linearDepth(const in vec2 uv)
{
    float d = texture(depthSampler, uv, 0).x;
    return projectionMatrix[3][2] / (d + projectionMatrix[2][2]);
}

mat3 noised(const in vec3 normal, in vec2 uv)
{
    uv *= screenSize * samplerSizes[3];

    vec3 random = texture(ssaoNoiseSampler, uv).xyz;

    // orientation matrix
    vec3 t = normalize(random - normal * dot(random, normal));
    vec3 b = cross(normal, t);

    return mat3(t, b, normal);
}

float ssao(float depth, vec3 normal)
{
    vec4 eye = (projectionInverseMatrix * vec4(2.0*(v_uv - vec2(0.5)), 1.0, 1.0));
    eye.xyz /= eye.w;
    eye.xyz /= farZ;
    // eye has a z of -1 here

    vec3 origin = eye.xyz * depth;

    vec3 viewNormal = normalMatrix * normal;

    // randomized orientation matrix for hemisphere based on face normal
    mat3 m = noised(viewNormal, v_uv);

    float ao = 0.0;

    for (float i = 0.0; i < samplerSizes[0]; ++i)
    {
        vec3 s = m * texture(ssaoKernelSampler, i * samplerSizes[1]).xyz;

        s *= 2.0 * ssaoRadius;
        s += origin;

        vec4 s_offset = projectionMatrix * vec4(s, 1.0);
        s_offset.xyz /= s_offset.w;

        s_offset.xy = s_offset.xy * 0.5 + 0.5;

        float sd = -linearDepth(s_offset.xy);

        float ndcRangeCheck = 1.0 - float(any(greaterThan(s_offset.xyz, vec3(1.0))) || any(lessThan(s_offset.xyz, vec3(0.0))));
        float rangeCheck = smoothstep(0.0, 1.0, ssaoRadius / abs(-origin.z + sd));
        ao += rangeCheck * ndcRangeCheck * float(sd > s.z);
    }

    return 1.0 - (ao * samplerSizes[1]);
}

void main()
{
    float d = linearDepth(v_uv);
    vec3 normal = normalize(texture(normalSampler, v_uv, 0).xyz);

    if (d > farZ)
        outColor = texture(colorSampler, v_uv).rgb;

    float ssao = ssao(d, normal);
    outColor = texture(colorSampler, v_uv).rgb * ssao;

    if (useReflections)
    {
        int samples = textureSize(reflectionKernelSampler, 0);
        vec3 reflectionOffset = texture(reflectionKernelSampler, currentFrame / float(samples)).rgb;
        vec3 reflectionNormal = normal + reflectionOffset * 0.1;
        reflectionNormal = normalize(reflectionNormal);

        vec3 worldPos = texture(worldPosSampler, v_uv).xyz;
        vec3 viewPos = worldToCamera(worldPos);
        vec3 worldViewDir = normalize(worldPos - cameraEye);
        vec3 reflectDir = reflect(worldViewDir, reflectionNormal);
        vec3 viewReflectDir = normalize(normalMatrix * reflectDir);

        float maxDist = farZ / 40;

        vec2 hitPixel;
        float strength;
        bool hit = traceScreenSpaceRay(viewPos, viewReflectDir, worldPosSampler, zThickness, pixelPerStep, maxTraceSteps, maxDist, hitPixel, strength);

        if (hit)
        {
            // fade out reflection torwards steep angles
            float reflectAngleFactor = 1.0 - 1.5 * max(0.0, dot(normal, reflectDir));
            reflectAngleFactor = clamp(reflectAngleFactor, 0.0, 1.0);

            float reflectMaterialFactor = texture(reflectSampler, v_uv).r;
            float reflectDistanceFactor = clamp(strength * 6, 0.0, 1.0);
            outColor = mix(outColor, texture(colorSampler, hitPixel).rgb * ssao, reflectDistanceFactor * reflectAngleFactor * reflectMaterialFactor);
        }
    }
}
