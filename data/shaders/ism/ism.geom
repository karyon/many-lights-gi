#version 430

#extension GL_ARB_shading_language_include : require
#include </data/shaders/common/random.glsl>
#include </data/shaders/common/floatpacking.glsl>
#include </data/shaders/ism/ism_utils.glsl>


layout(triangles) in;
layout(points, max_vertices = 1) out;

in vec3[] te_normal;
in vec3[] te_tessCoord;

out float g_normalRadius;

const int totalVplCount = 1024;
layout (std140, binding = 0) uniform packedVplBuffer_
{
    vec4 vplPositionNormalBuffer[totalVplCount];
};

layout (shared, binding = 0) buffer atomicBuffer_
{
	uint[1024] atomicCounter;
};

layout (r32ui, binding = 0) restrict uniform uimage3D softrenderBuffer;
layout (rgba32f, binding = 1) restrict writeonly uniform imageBuffer pointBuffer;

uniform ivec2 viewport;
uniform float zFar;

uniform bool usePushPull = true;

uniform int vplStartIndex = 0;
uniform int vplEndIndex = totalVplCount;
uniform bool scaleISMs = false;
uniform bool pointsOnlyIntoScaledISMs = false;

int vplCount = vplEndIndex - vplStartIndex;
int sampledVplCount = pointsOnlyIntoScaledISMs ? vplCount : totalVplCount;
int ismCount = (scaleISMs) ? vplCount : totalVplCount;
int ismIndices1d = int(pow(2, ceil(log2(ismCount) / 2))); // next even power of two
int vplIdOffset = pointsOnlyIntoScaledISMs ? vplStartIndex : 0;


void main()
{
    vec3 position = (gl_in[0].gl_Position.xyz + gl_in[1].gl_Position.xyz + gl_in[2].gl_Position.xyz) / 3;
    vec3 barycentricCoord = (te_tessCoord[0] + te_tessCoord[1] + te_tessCoord[2]) / 3;

    float maxdist = max(max(length(position - gl_in[0].gl_Position.xyz), length(position - gl_in[1].gl_Position.xyz)), length(position - gl_in[2].gl_Position.xyz));

    // point represents ismCount other points. boost its area by ismCount, i.e. boost its radius by sqrt(ismCount).
    float pointWorldRadius = maxdist * sqrt(ismCount);
    pointWorldRadius = min(pointWorldRadius, 15.0);
    uint g_normalRadius2 = pack4UNToUint(vec4(te_normal[0] * 0.5 + 0.5, pointWorldRadius / 15));

    vec3 seed = barycentricCoord.xyz + (gl_PrimitiveIDIn % 4096) / 4096.0;
    // seed = position.xyz;
    int base = int(random(seed) * sampledVplCount);

    if(usePushPull) {
        uint counter = atomicAdd(atomicCounter[base], 1);
        imageStore(pointBuffer, base * (imageSize(pointBuffer).x / sampledVplCount) + int(counter), vec4(position.xyz, uintBitsToFloat(g_normalRadius2)));
        return;
    }

    int vplID;
    vec3 vplNormal;
    vec3 positionRelativeToCamera;
    bool found = false;
    for(int i = 0; i < 1; i++) {
        int vplID2 = (base + i) % sampledVplCount;
        // vplID = int(counter) % 1024;
        if (pointsOnlyIntoScaledISMs)
            vplID2 += vplIdOffset;

        vec4 foo = vplPositionNormalBuffer[vplID2];
        vec3 vplPosition = foo.xyz;
        vec3 vplNormal2 = unpack3SNFromFloat(foo.w);

        vec3 positionRelativeToCamera2 = position.xyz - vplPosition;

        bool cull = dot(vplNormal2, positionRelativeToCamera2) < 0 || dot(te_normal[0], -positionRelativeToCamera2) < 0;
        found = found || !cull;
        if (!cull) {
            vplID = vplID2;
            vplNormal = vplNormal2;
            positionRelativeToCamera = positionRelativeToCamera2;
        }
    }
    if (!found)
        return;

    // paraboloid projection
    float distToCamera = length(positionRelativeToCamera);
    float ismIndex = scaleISMs ? float(vplID) - vplStartIndex : vplID;
    vec3 v = paraboloid_project(positionRelativeToCamera, distToCamera, vplNormal, zFar, ismIndex, ismIndices1d, true);

    vec3 normalPositionRelativeToCamera = positionRelativeToCamera + te_normal[0] * 0.1;
    float normalDist = length(normalPositionRelativeToCamera);
    vec3 normalV = paraboloid_project(normalPositionRelativeToCamera, normalDist, vplNormal, zFar, ismIndex, ismIndices1d, true);


    float ismSize = 64;
    float pointsPerMeter = 1.0 / (maxdist * 2.0);
    float pointSize = 1.0 / pointsPerMeter / distToCamera / 3.14 * viewport.x; // approximation that breaks especially for near points.
    pointSize *= 1.0;
    float maximumPointSize = 15.0;
    pointSize = min(pointSize, maximumPointSize);

    if (usePushPull) {
        v.xy *= imageSize(softrenderBuffer).xy;
        v.z *= 500000;
        uint original = imageAtomicMin(softrenderBuffer, ivec3(v.xy, 0), uint(v.z));

        if (original > uint(v.z)) {
            float radius = pointSize / 2;
            radius *= 1.3; // boost radius a bit to make circle area match the point rendering square area
            radius = min(radius, 15);
            uint g_normalRadius = pack4UNToUint(vec4(te_normal[0] * 0.5 + 0.5, radius / 15.0));
            // potential race condition here. two threads write into depth, and then both, in a different order, write into attributes.
            // largely solved in compute shader version
            imageStore(softrenderBuffer, ivec3(v.xy, 1), uvec4(g_normalRadius, 0, 0, 0));
        }
    }
    else {
        // to tex and NDC coords
        v.xy = v.xy * 2.0 - 1.0;
        v.z = v.z * 2.0 - 1.0;

        gl_Position = vec4(v, 1.0);

        gl_PointSize = pointSize;
        // gl_PointSize = 1;
        EmitVertex();
    }
}
