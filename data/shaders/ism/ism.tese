#version 430

layout (triangles, equal_spacing) in;

in vec3[] tc_normal;
out vec3 te_normal;

vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)
{
    return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
}

void main()
{
    gl_Position.xyz = interpolate3D(
        gl_in[0].gl_Position.xyz,
        gl_in[1].gl_Position.xyz,
        gl_in[2].gl_Position.xyz
    );
    gl_Position.w = 1.0;
    te_normal = tc_normal[0];
}
