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

#endif
