#version 330

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

uniform mat4 transforms[6];

out vec3 g_direction;

void main()
{
    for (int i = 0; i < 6; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            gl_Layer = i;
            g_direction = gl_in[j].gl_Position.xyz;
            gl_Position = transforms[i] * gl_in[j].gl_Position;

            EmitVertex();
        }
        EndPrimitive();
    }
}
