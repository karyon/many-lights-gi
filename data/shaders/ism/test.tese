#version 430

layout (triangles, point_mode, equal_spacing) in;

in vec3[] tc_normal;
out vec3 te_normal;
out vec3 te_tessCoord;



uniform mat4 modelView;
uniform mat4 projection;
uniform vec2 ndcOffset;
uniform vec2 cocPoint;
uniform float focalDist;

uniform mat4 biasedShadowTransform;

vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)
{
    return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
}

void main()
{
    gl_Position = projection * modelView * vec4(interpolate3D(
        gl_in[0].gl_Position.xyz,
        gl_in[1].gl_Position.xyz,
        gl_in[2].gl_Position.xyz
    ), 1.0);

    gl_PointSize = (gl_Position.z / gl_Position.w - 0.97) * 200;
    // gl_PointSize = 5;
    //gl_Position.w = 1.0;
    //te_normal = tc_normal[0];
    //te_tessCoord = gl_TessCoord;
}
