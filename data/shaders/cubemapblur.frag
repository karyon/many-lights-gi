#version 330

in vec3 g_direction;

layout (location = 0) out vec3 out_color;

uniform samplerCube shadowMap;
uniform vec2 direction;
uniform float sizeFactor;
uniform int kernelsize;

int calcFace(in vec3 direction)
{
    float x = abs(dot(direction, vec3(1,0,0)));
    float y = abs(dot(direction, vec3(0,1,0)));
    float z = abs(dot(direction, vec3(0,0,1)));

    if (x > y && x > z)
    {
        return 0;
    }
    if (y > z)
    {
        return 1;
    }
    return 2;
}

vec3 blurDirection(in vec2 offset, in int face)
{
    switch (face)
    {
        case 0: return vec3(0, offset);
        case 1: return vec3(offset.x, 0, offset.y);
        case 2: return vec3(offset, 0);
    }
}

void main()
{
    float sampleLength = 1.0 * sizeFactor; // sampleLength is the distance of 1 texel

    vec3 sampleDirection = normalize(g_direction);
    int face = calcFace(sampleDirection);

    out_color = vec3(0.0);
    int count = 0;
    for (int x = -kernelsize; x <= kernelsize; ++x)
    {
    	++count;
    	out_color += texture(shadowMap, sampleDirection + blurDirection(vec2(x) * direction, face) * sampleLength).rgb;
    }

    out_color.rgb /= count;
}
