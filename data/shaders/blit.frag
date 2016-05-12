#version 330

in vec2 v_uv;

out vec3 outColor;

uniform sampler2D someBuffer;
uniform bool singleChannel;

void main()
{
    outColor = texture(someBuffer, v_uv).rgb;

    if (singleChannel)
        outColor = vec3(outColor.r);
}
