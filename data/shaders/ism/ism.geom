#version 430

#extension GL_ARB_shading_language_include : require
#include </data/shaders/common/random.glsl>
#include </data/shaders/common/floatpacking.glsl>
#include </data/shaders/ism/ism_utils.glsl>


layout(points) in;
layout(points, max_vertices = 1) out;

out float g_radius;

const int totalVplCount = 1024;
layout (std140, binding = 0) uniform packedVplBuffer_
{
    vec4 vplPositionNormalBuffer[totalVplCount];
};

layout (std140, binding = 0) buffer atomicBuffer_
{
	uint atomicCounter;
};

uniform ivec2 viewport;
uniform float zFar;

uniform int vplStartIndex = 0;
uniform int vplEndIndex = totalVplCount;
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

    uint counter = atomicAdd(atomicCounter, 1);

    int vplID = (int(rand(position.xyz) * sampledVplCount) + gl_PrimitiveIDIn) % sampledVplCount;
    // vplID = int(counter) % 1024;
    if (pointsOnlyIntoScaledISMs)
        vplID += vplIdOffset;

    vec3 vplPosition = vplPositionNormalBuffer[vplID].xyz;
    vec3 vplNormal = Unpack3PNFromFP32(vplPositionNormalBuffer[vplID].w) * 2.0 - 1.0;

    vec3 positionRelativeToCamera = position.xyz - vplPosition;

    // paraboloid projection
    float distToCamera = length(positionRelativeToCamera);
    float ismIndex = scaleISMs ? float(vplID) - vplStartIndex : vplID;
    vec3 v = paraboloid_project(positionRelativeToCamera, distToCamera, vplNormal, zFar, ismIndex, ismIndices1d, true);
    bool cull = v.z <= 0.0;

    // to tex and NDC coords
    v.xy = v.xy * 2.0 - 1.0;
    v.z = v.z * 2.0 - 1.0;

    gl_Position = vec4(v, 1.0);

    float pointsPerMeter = 5.0; // actual number unknown, needs to be calculated during tesselation
    float pointSize = 1.0 / pointsPerMeter / distToCamera / 3.14 * viewport.x; // approximation that breaks especially for near points.
    float maximumPointSize = 15.0;
    pointSize = min(pointSize, maximumPointSize);

    g_radius = pointSize;
    pointSize = 1;
    pointSize *= float(!cull);
    gl_PointSize = pointSize;
    EmitVertex();
}
