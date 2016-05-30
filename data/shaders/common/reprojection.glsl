#ifndef REPROJECTION
#define REPROJECTION

// returns values in [nearZ:farZ]
// http://www.derschmale.com/2014/01/26/reconstructing-positions-from-the-depth-buffer/
float linearDepth(sampler2D depthSampler, vec2 uv, mat4 projectionMatrix)
{
    float z_b = texture(depthSampler, uv, 0).x;
    float z_n = 2.0 * z_b - 1.0;
    float A = projectionMatrix[2][2];
    float B = projectionMatrix[3][2];
    float z_e = -B / (A + z_n);
    return z_e;
}

#endif
