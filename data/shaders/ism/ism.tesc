#version 410

layout (vertices = 3) out;

in vec3[] v_normal;
out vec3[] tc_normal;


uniform float tessLevelFactor = 2.0f;

void main(void)
{
    float length0 = length(gl_in[1].gl_Position - gl_in[2].gl_Position);
    float length1 = length(gl_in[2].gl_Position - gl_in[0].gl_Position);
    float length2 = length(gl_in[0].gl_Position - gl_in[1].gl_Position);
    gl_TessLevelOuter[0] = length0 * tessLevelFactor;
    gl_TessLevelOuter[1] = length1 * tessLevelFactor;
    gl_TessLevelOuter[2] = length2 * tessLevelFactor;

    gl_TessLevelInner[0] = max(max(gl_TessLevelOuter[0], gl_TessLevelOuter[1]), gl_TessLevelOuter[2]);
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    tc_normal[gl_InvocationID] = v_normal[gl_InvocationID];
}
