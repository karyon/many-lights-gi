#ifndef FRAGMENT_DISCARD
#define FRAGMENT_DISCARD

bool fragmentDiscard(const in float alpha, const int sampleID, const in sampler2D masksTexture, const in float rand)
{
    if (alpha >= 1.0)
    {
        return false;
    }

    ivec2 index = ivec2(rand * 1023.0, int(alpha * 255.0));
    uint mask = uint(texelFetch(masksTexture, index, 0).r * 255.0);

    uint sampleBit = 1u << sampleID;

    return (mask & sampleBit) != sampleBit;
}

#endif
