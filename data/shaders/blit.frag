#version 330

in vec2 v_uv;

out vec3 outColor;

uniform sampler2D someBuffer;
uniform bool singleChannel;
uniform int mipLevel;

void main()
{
    outColor = textureLod(someBuffer, v_uv, mipLevel).rgb;

    if (singleChannel)
        outColor = vec3(outColor.r);
}
