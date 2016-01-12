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
    float variance = max(moments.y - moments.x*moments.x, -0.001);
    float d = compare - moments.x;
    float p_max = linstep(0.3, 1.0, variance / (variance + d*d));
    return clamp(max(p, p_max), 0.0, 1.0);
}

float omnishadowmapComparisonVSM(samplerCube shadowmap, vec3 worldPos, vec3 worldLightPos)
{
    vec3 lightDirection = worldPos - worldLightPos;
    float dist = length(lightDirection);
    vec3 texel = texture(shadowmap, lightDirection).rgb;
    vec2 moments = texel.rg;
    float alpha = texel.b;
    float transparency = 1.0 - alpha;
    float vsm = VSM(moments, dist);
    return transparency + (vsm - transparency) * vsm; // transform from [0.0;1.0] to [transparency:1.0]
}

float omnishadowmapComparison(samplerCube shadowmap, vec3 worldPos, vec3 worldLightPos)
{
    vec3 lightDirection = worldPos - worldLightPos;
    float dist = length(lightDirection);
    float comp = texture(shadowmap, lightDirection).r;
    return float(dist - EPSILON  < comp);
}

#endif
