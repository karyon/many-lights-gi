#version 330

in vec2 v_uv;

out vec3 outColor;

uniform sampler2D accumBuffer;
uniform sampler2D frameBuffer;
uniform float weight;

void main()
{
    vec3 a = texture(accumBuffer, v_uv).rgb;
    vec3 c = texture(frameBuffer, v_uv).rgb;

    outColor = mix(a, c, weight);
}
