#version 330

in float g_radius;

layout(location = 0) out float outRadius;

void main()
{
    outRadius = g_radius;
}
