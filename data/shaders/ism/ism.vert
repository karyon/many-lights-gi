#version 330

layout(location = 0) in vec3 a_vertex;

void main()
{
    vec4 vertex = vec4(a_vertex, 1.0);

    gl_Position = vertex;
}
