#version 330

#extension GL_ARB_shading_language_include : require
#include </data/shaders/common/shadowmapping.glsl>

in vec2 v_uv;
in vec3 v_viewRay;

out vec3 outColor;

uniform sampler2D diffuseSampler;
uniform sampler2D specularSampler;
uniform sampler2D faceNormalSampler;
uniform sampler2D normalSampler;
uniform sampler2D depthSampler;
uniform sampler2D worldPosSampler;
uniform sampler2D shadowmap;

uniform mat3 normalMatrix;
uniform mat4 projectionMatrix;
uniform mat4 projectionInverseMatrix;
uniform mat4 biasedLightViewProjectionMatrix;
uniform vec3 worldLightPos;
uniform vec3 cameraEye;
uniform mat4 viewMatrix;
uniform mat4 viewInvertedMatrix;
uniform float zFar;
uniform float zNear;
uniform vec2 screenSize;

const float ambientFactor = 0.25;
const float specularFactor = 0.75;

// returns values in [nearZ:farZ]
float linearDepth(const in vec2 uv)
{
    float d = texture(depthSampler, uv, 0).x;
    return projectionMatrix[3][2] / (d + -(zFar / (zFar - zNear)));
}

void main()
{
    float d = linearDepth(v_uv);
    vec3 N = normalize(texture(normalSampler, v_uv, 0).xyz);

    vec3 viewCoord = d * v_viewRay / zFar / 2.0; // unclear why the 2.0 is necessary
    vec3 worldCoord = (viewInvertedMatrix * vec4(viewCoord, 1.0)).xyz;

    vec3 L = normalize(worldLightPos - worldCoord);
    vec3 V = normalize(cameraEye - worldCoord);
    vec3 H = normalize(L + V);
    float ndotl = dot(N, L);
    float ndotH = dot(N, H);

    vec4 scoord = biasedLightViewProjectionMatrix * vec4(worldCoord, 1.0);


    float shadowFactor = shadowmapComparisonVSM(shadowmap, scoord.xy/scoord.w, worldCoord, worldLightPos);
    shadowFactor *= step(0.0, sign(scoord.w));


    vec3 diffuseColor = texture(diffuseSampler, v_uv, 0).xyz;
    vec3 specularColor = texture(specularSampler, v_uv, 0).xyz;
    vec3 ambientTerm = ambientFactor * diffuseColor;
    vec3 diffuseTerm = diffuseColor * max(0.0, ndotl) * shadowFactor;
    vec3 specularTerm = specularFactor * specularColor * pow(max(0.0, ndotH), 20.0) * shadowFactor;

    outColor = ambientTerm + diffuseTerm + specularTerm;
}
