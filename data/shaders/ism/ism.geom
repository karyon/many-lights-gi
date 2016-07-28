#version 430

#extension GL_ARB_shading_language_include : require
#include </data/shaders/common/random.glsl>
#include </data/shaders/common/floatpacking.glsl>
#include </data/shaders/ism/ism_utils.glsl>


layout(triangles) in;
layout(points, max_vertices = 1) out;

in vec3[] te_normal;

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
int ismIndices1d = int(ceil(sqrt(ismCount)));
int vplIdOffset = pointsOnlyIntoScaledISMs ? vplStartIndex : 0;

const float infinity = 1. / 0.;


void main()
{
    vec3 position = (gl_in[0].gl_Position.xyz + gl_in[1].gl_Position.xyz + gl_in[2].gl_Position.xyz) / 3;



    float maxdist = max(max(length(position - gl_in[0].gl_Position.xyz), length(position - gl_in[1].gl_Position.xyz)), length(position - gl_in[2].gl_Position.xyz));

    uint g_normalRadius2 = Pack4PNToUint(vec4(te_normal[0] * 0.5 + 0.5, maxdist / 15.0));

    int base = (int((random(position.xyz)) * 1024));
    float r = random(position.xyz);
    uint counter = atomicAdd(atomicCounter[base], 1);

    // imageStore(softrenderBuffer, ivec3(base, counter, 0), uvec4(g_normalRadius, 0, 0, 0));
    if(usePushPull) {
        imageStore(pointBuffer, base * 4096 + int(counter), vec4(position.xyz, uintBitsToFloat(g_normalRadius2)));
        return;

    }

    int vplID;
    vec3 vplNormal;
    vec3 positionRelativeToCamera;
    bool found = false;
    int i;
    // int base = (int(random(position.xyz) * sampledVplCount));
    for(i = 0; i < 1; i++) {
        int vplID2 = (base + i) % sampledVplCount;
        // vplID = int(counter) % 1024;
        if (pointsOnlyIntoScaledISMs)
            vplID2 += vplIdOffset;

        vec4 foo = vplPositionNormalBuffer[vplID2];
        vec3 vplPosition = foo.xyz;
        vec3 vplNormal2 = Unpack3PNFromFP32(foo.w) * 2.0 - 1.0;

        vec3 positionRelativeToCamera2 = position.xyz - vplPosition;

        bool cull = dot(vplNormal2, positionRelativeToCamera2) < 0 || dot(te_normal[0], -positionRelativeToCamera2) < 0;
        found = found || !cull;
        if (!cull) {
            vplID = vplID2;
            vplNormal = vplNormal2;
            positionRelativeToCamera = positionRelativeToCamera2;
            // break;
        }
    }
    if (!found)
        return;

    // vec4 foo1 = vplPositionNormalBuffer[vplID];
    // vec4 foo2 = vplPositionNormalBuffer[vplID];
    // vec4 foo3 = vplPositionNormalBuffer[vplID];
    // positionRelativeToCamera += (foo1.xyz + foo2.xyz + foo3.xyz + Unpack3PNFromFP32(foo1.w) + Unpack3PNFromFP32(foo2.w) + Unpack3PNFromFP32(foo3.w)) * 0.00001;
    // positionRelativeToCamera.x += (dot(foo1.xyz, foo2.xyz) + dot(foo3.xyz, Unpack3PNFromFP32(foo3.w) )) * 0.00001;

    for(i = 0; i < 0; i++) {
        int vplID2 = (base + i) % sampledVplCount;
        // vplID = int(counter) % 1024;
        if (pointsOnlyIntoScaledISMs)
            vplID2 += vplIdOffset;

        vec4 foo = vplPositionNormalBuffer[vplID2];
        vec3 vplPosition = foo.xyz;
        vec3 vplNormal2 = Unpack3PNFromFP32(foo.w) * 2.0 - 1.0;

        vec3 positionRelativeToCamera2 = position.xyz - vplPosition;

        bool cull = dot(vplNormal2, positionRelativeToCamera2) < 0 || dot(te_normal[0], -positionRelativeToCamera2) < 0;
        // found = found || !cull;
        positionRelativeToCamera += min(positionRelativeToCamera2, 1) * 0.0001;
        // if (!cull) {
        //     vplID = vplID2;
        //     vplNormal = vplNormal2;
        //     positionRelativeToCamera = positionRelativeToCamera2;
        //     // break;
        // }
    }



    // vplNormal = Unpack3PNFromFP32(vplPositionNormalBuffer[vplID].w) * 2.0 - 1.0;
    // vec3 vplPosition = vplPositionNormalBuffer[vplID].xyz;
    // positionRelativeToCamera = position.xyz - vplPosition;

    // paraboloid projection
    float distToCamera = length(positionRelativeToCamera);
    float ismIndex = scaleISMs ? float(vplID) - vplStartIndex : vplID;
    vec3 v = paraboloid_project(positionRelativeToCamera, distToCamera, vplNormal, zFar, ismIndex, ismIndices1d, true);

    vec3 normalPositionRelativeToCamera = positionRelativeToCamera + te_normal[0] * 0.1;
    float normalDist = length(normalPositionRelativeToCamera);
    vec3 normalV = paraboloid_project(normalPositionRelativeToCamera, normalDist, vplNormal, zFar, ismIndex, ismIndices1d, true);


    // to tex and NDC coords
    v.xy = v.xy * 2.0 - 1.0;
    v.z = v.z * 2.0 - 1.0;

    gl_Position = vec4(v, 1.0);

    float pointsPerMeter = 1.0 / (maxdist * 2.0);
    float pointSize = 1.0 / pointsPerMeter / distToCamera / 3.14 * viewport.x; // approximation that breaks especially for near points.
    pointSize *= 1.0;
    float maximumPointSize = 15.0;
    pointSize = min(pointSize, maximumPointSize);

    if (usePushPull) {
        // v.xy = v.xy * 0.5 + 0.5;
        // v.xy *= 2048;
        // v.z  = v.z * 0.5 + 0.5;
        // v.z *= 5000;
        // uint original = imageAtomicMin(softrenderBuffer, ivec3(v.xy, 0), uint(v.z));
        //
        // if (original > uint(v.z)) {
        //     float radius = pointSize / 2;
        //     radius *= 1.3; // boost radius a bit to make circle area match the point rendering square area
        //     uint g_normalRadius = Pack4PNToUint(vec4(te_normal[0] * 0.5 + 0.5, radius / 15.0));
        //     // potential race condition here. two threads write into depth, and then both, in a different order, write into attributes.
        //     // but this should almost never happen in practice, right?
        //     imageStore(softrenderBuffer, ivec3(v.xy, 1), uvec4(g_normalRadius, 0, 0, 0));
        // }
    }
    // else {
        gl_PointSize = pointSize;
        gl_PointSize = 1;
        EmitVertex();
    // }
}
