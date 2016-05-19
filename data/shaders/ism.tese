#version 410

layout (triangles, equal_spacing, point_mode) in;


uniform sampler2D rsmFaceNormalSampler;
uniform sampler2D rsmDepthSampler;
uniform mat4 modelView;
uniform mat4 biasedLightViewProjectionInverseMatrix;

const int ism_count1d = 16;
const int ism_count = ism_count1d * ism_count1d;

vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)
{
    return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
}

//based on glm matrix_transform.inl
mat4 lookAtRH
(
    vec3 eye,
    vec3 center,
    vec3 up
)
{
    vec3 f = normalize(center - eye);
    vec3 s = normalize(cross(f, up));
    vec3 u = cross(s, f);

    mat4 Result = mat4(1);
    Result[0][0] = s.x;
    Result[1][0] = s.y;
    Result[2][0] = s.z;
    Result[0][1] = u.x;
    Result[1][1] = u.y;
    Result[2][1] = u.z;
    Result[0][2] =-f.x;
    Result[1][2] =-f.y;
    Result[2][2] =-f.z;
    Result[3][0] =-dot(s, eye);
    Result[3][1] =-dot(u, eye);
    Result[3][2] = dot(f, eye);
    return Result;
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

    vec3 vplNormal = texture(rsmFaceNormalSampler, ismIndexFloat).xyz * 2.0 - 1.0;
    float rsm_depth = texture(rsmDepthSampler, ismIndexFloat).r;

    vec4 vplWorldcoords = biasedLightViewProjectionInverseMatrix * vec4(ismIndexFloat, rsm_depth, 1.0);
    vplWorldcoords.xyz /= vplWorldcoords.w;

    mat4 vpl_view = lookAtRH(vplWorldcoords.xyz, vplWorldcoords.xyz + vplNormal, vec3(0.0, 1.0, 0.0));

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
