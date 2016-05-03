#ifndef REPROJECTION
#define REPROJECTION

// returns values in [nearZ:farZ]
float linearDepth(sampler2D depthSampler, vec2 uv, float projectionA, float projectionB)
{
    float d = texture(depthSampler, uv, 0).x;
    return projectionA / (d - projectionB);
}

#endif
