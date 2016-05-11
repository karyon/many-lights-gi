#version 410

layout (triangles, equal_spacing, point_mode) in;


uniform mat4 modelView;


vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)
{
    return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
}

void main()
{
    gl_Position = gl_in[0].gl_Position;

    vec4 v = modelView * gl_Position;

    float dist = length(v);
    v.xyz /= dist;

    v.z = 1.0 - v.z;
    v.xy /= v.z;
    v.z = dist / (2000.0);

    gl_Position = v;

    gl_PointSize = 2;
}
