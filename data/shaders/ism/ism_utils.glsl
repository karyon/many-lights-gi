#ifndef ISM_UTILS
#define ISM_UTILS

//based on glm matrix_transform.inl
mat3 lookAtRH(vec3 normalizedNormal)
{
    vec3 up = vec3(0.0, 1.0, 0.01);
    vec3 f = normalizedNormal;
    vec3 s = normalize(cross(f, up));
    vec3 u = cross(s, f);

    return transpose(mat3(s, u, -f));
}

vec3 paraboloid_project(vec3 positionRelativeToCamera, float distToCamera, vec3 vplNormal, float zFar, float ismIndex, int ismIndices1d, bool preserveSign)
{
    mat3 vplView = lookAtRH(vplNormal);

    vec3 v = vplView * positionRelativeToCamera;
    float signOfV = sign(v.z);

    // paraboloid projection
    v.xyz /= distToCamera;
    v.z = 1.0 - v.z;
    v.xy /= v.z;
    v.z = distToCamera / zFar;
    if (preserveSign)
        v.z *= -signOfV;

    // scale and bias to texcoords
    v.xy += 1.0;
    v.xy /= 2.0;

    // offset to respective ISM
    float y = int(ismIndex / ismIndices1d);
    ivec2 ismIndex2d = ivec2(int(ismIndex - y * ismIndices1d) , y);
    v.xy += vec2(ismIndex2d);
    v.xy /= ismIndices1d;
    return v;
}

#endif
