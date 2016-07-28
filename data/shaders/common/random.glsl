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

// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

// Compound versions of the hashing algorithm I whipped together.
uint hash( uvec2 v ) { return hash( v.x ^ hash(v.y)                         ); }
uint hash( uvec3 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z)             ); }
uint hash( uvec4 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }

// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );       // Range [1:2]
    return f - 1.0;                  // Range [-1:1]
}

// Pseudo-random value in half-open range [0:1].
float random( float x ) { return floatConstruct(hash(floatBitsToUint(x))); }
float random( float x, float y ) { return floatConstruct(hash(floatBitsToUint(vec2(x, y)))); }
float random( vec2  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec3  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec4 v ) { return floatConstruct(hash(floatBitsToUint(v))); }

#endif
