#ifndef TRANSFORMATIONS
#define TRANSFORMATIONS

vec4 subpixelShift(mat4 projection, vec4 viewVertex, vec2 ndcOffset)
{
    vec4 shiftedVertex = projection * viewVertex;
    shiftedVertex.xy += ndcOffset * shiftedVertex.w;
    return shiftedVertex;
}

vec4 depthOfField(mat4 modelView, vec4 worldPos, vec2 cocPoint, float focalDist)
{
    vec4 viewVertex = modelView * worldPos;
    viewVertex.xy += cocPoint * (viewVertex.z + focalDist);
    return viewVertex;
}

#endif
