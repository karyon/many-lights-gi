#version 430

#extension GL_ARB_shading_language_include : require
#include </data/shaders/common/random.glsl>
#include </data/shaders/common/ism_utils.glsl>

uniform ivec2 viewport;
uniform float zFar;

struct VPL {
    vec4 position;
    vec4 normal;
    vec4 color;
};

const int totalVplCount = 256;
layout (std140, binding = 0) uniform vplBuffer_
{
    VPL vplBuffer[totalVplCount];
};


layout(points) in;
layout(points, max_vertices = 1) out;



uniform int vplStartIndex = 0;
uniform int vplEndIndex = 256;
uniform bool scaleISMs = false;
uniform bool pointsOnlyIntoScaledISMs = false;

int vplCount = vplEndIndex - vplStartIndex;
int sampledVplCount = pointsOnlyIntoScaledISMs ? vplCount : totalVplCount;
int ismCount = (scaleISMs) ? vplCount : totalVplCount;
int ismIndices1d = int(ceil(sqrt(ismCount)));
int vplIdOffset = pointsOnlyIntoScaledISMs ? vplStartIndex : 0;

const float infinity = 1. / 0.;


void main()
{
    vec4 position = gl_in[0].gl_Position;

    int vplID = (int(rand(position.xyz) * sampledVplCount) + gl_PrimitiveIDIn) % sampledVplCount;
    if (pointsOnlyIntoScaledISMs)
        vplID += vplIdOffset;


    int ismIndex = scaleISMs ? vplID - vplStartIndex : vplID;

    ivec2 ismIndex2d = ivec2(ismIndex % ismIndices1d, ismIndex / ismIndices1d);
    vec2 ismIndexFloat = vec2(ismIndex2d) / ismIndices1d;

    vec3 vplNormal = vplBuffer[vplID].normal.xyz;
    mat3 vplView = lookAtRH(vplNormal);
    vec3 vplPosition = vplBuffer[vplID].position.xyz;

    vec3 v = position.xyz - vplPosition;
    v = vplView * v;

    // culling
    if (v.z > 0.0) {
        v.z = infinity;
        // return; // makes it slower
    }

    float dist = length(v);
    v.xyz /= dist;

    v.z = 1.0 - v.z;
    v.xy /= v.z;
    v.z = dist / zFar;
    v.z = v.z * 2.0 - 1.0; // to [-1; 1] to match usual NDC coordinates

    v.xy += 1.0;
    v.xy /= 2.0;
    v.xy /= ismIndices1d;
    v.xy += ismIndexFloat;
    v.xy *= 2.0;
    v.xy -= 1.0;

    gl_Position = vec4(v, 1.0);

    float pointsPerMeter = 20.0; // actual number unknown, needs to be calculated during tesselation
    float pointSize = 1.0 / pointsPerMeter / dist / 3.14 * viewport.x; // approximation that breaks especially for near points.
    float maximumPointSize = 10.0;
    pointSize = min(pointSize, maximumPointSize);

    gl_PointSize = pointSize;
    EmitVertex();
}
