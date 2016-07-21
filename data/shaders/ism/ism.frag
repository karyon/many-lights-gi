#version 330

in float g_normalRadius;

layout(location = 0) out float outNormalRadius;

void main()
{
    outNormalRadius = g_normalRadius;
}
