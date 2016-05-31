#version 430

#extension GL_ARB_shading_language_include : require
#include </data/shaders/common/random.glsl>

uniform ivec2 viewport;

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


layout(points) in;
layout(points, max_vertices = 1) out;

const int ism_count1d = 16;
const int ism_count = ism_count1d * ism_count1d;
const float infinity = 1. / 0.;


void main()
{
    vec4 position = gl_in[0].gl_Position;

    int ismID = (int(rand(position.xyz) * ism_count) + gl_PrimitiveIDIn) % ism_count;
    //ismID += 146;

    ivec2 ismIndex = ivec2(ismID % ism_count1d, ismID / ism_count1d);
    ismIndex %= ism_count1d; //for debugging
    vec2 ismIndexFloat = vec2(ismIndex) / ism_count1d;

    mat4 vpl_view = vplBuffer[ismID].viewMatrix;

    vec4 v = vpl_view * position;

    // culling
    if (v.z > 0.0) {
        v.z = infinity;
        // return; // makes it slower
    }

    float dist = length(v);
    v.xyz /= dist;

    v.z = 1.0 - v.z;
    v.xy /= v.z;
    v.z = dist / (2000.0);
    v.z = v.z * 2.0 - 1.0; // to [-1; 1] to match usual NDC coordinates

    v.xy += 1.0;
    v.xy /= 2.0;
    v.xy /= ism_count1d;
    v.xy += ismIndexFloat;
    v.xy *= 2.0;
    v.xy -= 1.0;

    gl_Position = v;
    float pointSize = dist /= 2000;
    pointSize = 1.0 - pointSize;
    pointSize *= pointSize;
    float arbitraryPointSizeDivisor = 200;
    float arbitraryPointSizeMinimum = 2;
    float maximumPointSize = viewport.x / arbitraryPointSizeDivisor;
    pointSize = mix(arbitraryPointSizeMinimum, maximumPointSize, pointSize);

    gl_PointSize = pointSize;
    EmitVertex();
}
