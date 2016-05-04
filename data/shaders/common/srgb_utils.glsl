#ifndef SRGB_UTILS
#define SRGB_UTILS

const bool linearRendering = true;

const float gamma = linearRendering ? 2.2 : 1.0;

float toLinear(float v)
{
    return pow(v, gamma);
}

vec3 toLinear(vec3 v)
{
    return pow(v, vec3(gamma));
}

float toSRGB(float v)
{
    return pow(v, 1.0 / gamma);
}

vec3 toSRGB(vec3 v)
{
    return pow(v, vec3(1.0 / gamma));
}

#endif
