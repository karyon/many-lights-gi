#version 330

in vec2 v_uv;

out vec3 outColor;

uniform sampler2D someBuffer;
uniform usampler2D softRenderBuffer;
uniform bool singleChannel;
uniform bool softRenderBufferActive;
uniform int mipLevel;

void main()
{
    if (softRenderBufferActive) {
        outColor = vec3(textureLod(softRenderBuffer, vec2(v_uv), mipLevel).r >> 8) / (1 << 24);
    } else {
        outColor = textureLod(someBuffer, v_uv, mipLevel).rgb;
    }

    if (singleChannel)
        outColor = vec3(outColor.r);
}
