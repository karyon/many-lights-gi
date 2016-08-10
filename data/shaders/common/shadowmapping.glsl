#ifndef SHADOWMAPPING
#define SHADOWMAPPING

#define EPSILON 0.001

float linstep(float low, float high, float v)
{
    return clamp((v-low)/(high-low), 0.0, 1.0);
}

float VSM(vec2 moments, float compare)
{
    float p = smoothstep(compare-0.001, compare, moments.x);
    float variance = max(moments.y - moments.x*moments.x, 0.001);
    float d = compare - moments.x;
    float p_max = linstep(0.1, 1.0, variance / (variance + d*d));
    return clamp(max(p, p_max), 0.0, 1.0);
}

float omnishadowmapComparisonVSM(samplerCube shadowmap, vec3 worldPos, vec3 worldLightPos)
{
    vec3 lightDirection = worldPos - worldLightPos;
    float dist = length(lightDirection);
    vec2 moments = texture(shadowmap, lightDirection).rg;
    return VSM(moments, dist);
}

float shadowmapComparisonVSM(sampler2D shadowmap, vec2 scoord, vec3 worldPos, vec3 worldLightPos)
{
    vec3 lightDirection = worldPos - worldLightPos;
    float dist = length(lightDirection);
    vec2 moments = texture(shadowmap, scoord).rg;
    return VSM(moments, dist);
}

float omnishadowmapComparison(samplerCube shadowmap, vec3 worldPos, vec3 worldLightPos)
{
    vec3 lightDirection = worldPos - worldLightPos;
    float dist = length(lightDirection);
    float comp = texture(shadowmap, lightDirection).r;
    return float(dist - EPSILON  < comp);
}

#endif
