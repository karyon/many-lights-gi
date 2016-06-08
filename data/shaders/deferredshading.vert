#version 140
#extension GL_ARB_explicit_attrib_location : require

uniform mat4 projectionInverseMatrix;

out vec2 v_uv;
out vec3 v_viewRay;

void main()
{
    v_uv = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    gl_Position = vec4(v_uv * vec2(2.0) + vec2(-1.0) , 0.0, 1.0);

    vec4 vertex_vs = projectionInverseMatrix * gl_Position;
    v_viewRay = vertex_vs.xyz / vertex_vs.w;
    v_viewRay /= v_viewRay.z;
}
