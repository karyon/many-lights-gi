#version 430

layout (triangles, equal_spacing, point_mode) in;

struct vpl {
    vec4 position;
    vec4 normal;
    vec4 color;
    mat4 viewMatrix;
};

layout (std140, binding = 0) uniform vplBuffer_
{
    vpl vplBuffer[256];
};

const int ism_count1d = 16;
const int ism_count = ism_count1d * ism_count1d;


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

    int ismID = gl_PrimitiveID % ism_count;
    ivec2 ismIndex = ivec2(ismID % ism_count1d, ismID / ism_count1d);
    vec2 ismIndexFloat = vec2(ismIndex) / ism_count1d;

    vec3 vplWorldcoords = vplBuffer[ismID].position.xyz;
    mat4 vpl_view = vplBuffer[ismID].viewMatrix;
    vec3 vplNormal = vplBuffer[ismID].normal.xyz;

    vec4 v = vpl_view * gl_Position;

    if (v.z > 0.0) {
        v.x = 90000;
    }

    float dist = length(v);
    v.xyz /= dist;

    v.z = 1.0 - v.z;
    v.xy /= v.z;
    v.z = dist / (2000.0);
    v.z = v.z * 2.0 - 1.0;

    v.xy += 1.0;
    v.xy /= 2.0;
    v.xy /= ism_count1d;
    v.xy += ismIndexFloat;
    v.xy *= 2.0;
    v.xy -= 1.0;


    gl_Position = v;

    gl_PointSize = 4;
}
