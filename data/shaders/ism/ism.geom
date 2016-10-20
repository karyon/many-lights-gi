#version 430

#extension GL_ARB_shading_language_include : require
#include </data/shaders/common/random.glsl>
#include </data/shaders/common/floatpacking.glsl>
#include </data/shaders/ism/ism_utils.glsl>


layout(triangles) in;
layout(points, max_vertices = 1) out;

in vec3[] te_normal;
in vec3[] te_tessCoord;

flat out ivec2 g_centerCoord;
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

layout (r32ui, binding = 0) restrict uniform uimage2D softrenderBuffer;
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

    vec3 seed = barycentricCoord.xyz + (gl_PrimitiveIDIn % 4096) / 4096.0;
    int base = int(random(seed) * sampledVplCount);

    if(usePushPull) {
        float pointWorldRadius = maxdist;
        // each point represents ismCount other points.
        // therefore boost its area by ismCount, i.e. boost its radius by sqrt(ismCount).
        pointWorldRadius *= sqrt(ismCount);
        float normalRadius = pack4UNToFloat(vec4(te_normal[0] * 0.5 + 0.5, pointWorldRadius / 25.0));

        uint counter = atomicAdd(atomicCounter[base], 1);
        int writeIndex = base * (imageSize(pointBuffer).x / sampledVplCount) + int(counter);
        imageStore(pointBuffer, writeIndex, vec4(position.xyz, normalRadius));
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


    float pointWorldRadius = maxdist;

    // as above, boost area by sqrt(ismCount).
    // commented out since this and the next line cancel each other out
    // pointWorldRadius *= sqrt(ismCount);

    // each ISM has only sqrt(ismCount) the area of the complete viewport
    // pointWorldRadius /= sqrt(ismCount);

    float pointSize = (pointWorldRadius * 2.0) / distToCamera / 3.14 * viewport.x; // approximation that breaks especially for near points.
    pointSize *= 1.0;
    float maximumPointSize = 15.0;
    pointSize = min(pointSize, maximumPointSize);

    if (usePushPull) {
        v.xy *= imageSize(softrenderBuffer).xy;
        v.z *= 1 << 24;
        // uint original = imageAtomicMin(softrenderBuffer, ivec2(v.xy), uint(v.z));

            float radius = pointSize / 2;
            radius *= 1.3; // boost radius a bit to make circle area match the point rendering square area

        uint currentDepthValue = uint(v.z) << 8;
        currentDepthValue |= uint(radius * 10);
        uint originalDepthValue = imageAtomicMin(softrenderBuffer, ivec2(v.xy), currentDepthValue);
    }
    else {
        g_centerCoord = ivec2(v.xy * viewport);

        // to tex and NDC coords
        v.xy = v.xy * 2.0 - 1.0;
        v.z = v.z * 2.0 - 1.0;

        gl_Position = vec4(v, 1.0);

        gl_PointSize = pointSize;
        // gl_PointSize = 1;
        EmitVertex();
    }
}
