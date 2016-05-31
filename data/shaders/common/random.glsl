#ifndef RANDOM
#define RANDOM

const float random_a = 12.9898;
const float random_b = 78.233;
const float random_c = 43758.5453;

float rand(const in vec2 coord)
{
    float dt= dot(coord.xy, vec2(random_a, random_b));
    float sn= mod(dt, 3.14);
    return fract(sin(sn) * random_c);
}

float rand(const in vec4 fragCoord, const in vec2 screenSize, const in vec3 worldCoord)
{
    vec2 normFragCoord = floor(fragCoord.xy) / screenSize * dot(worldCoord.x, worldCoord.y);
    return rand(normFragCoord.xy);
}

// accepts a 3D noise texture with two channels
// randSeed between 0.0 - 1.0
float rand(in sampler3D noiseTexture, const in vec3 worldCoord, const in float randSeed)
{
    return fract(rand(texture(noiseTexture, worldCoord).rg) + randSeed);
}

float rand(vec3 co)
{
    return fract(sin( dot(co.xyz ,vec3(12.9898,78.233,45.5432) )) * 43758.5453);
}

#endif
