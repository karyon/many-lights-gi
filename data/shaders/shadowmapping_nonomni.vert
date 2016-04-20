#version 330

layout(location = 0) in vec3 a_vertex;

out vec3 g_worldCoord;
out vec3 v_S;

uniform mat4 transform;


void main()
{
    vec4 vertex = vec4(a_vertex, 1.0);
    g_worldCoord = a_vertex;
    gl_Position = transform * vertex;
}
