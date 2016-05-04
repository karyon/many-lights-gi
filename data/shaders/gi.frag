#version 330

#extension GL_ARB_shading_language_include : require
#include </data/shaders/common/shadowmapping.glsl>
#include </data/shaders/common/reprojection.glsl>

in vec2 v_uv;
in vec3 v_viewRay;

out vec3 outColor;

uniform sampler2D faceNormalSampler;
uniform sampler2D depthSampler;
uniform sampler2D lightDiffuseSampler;
uniform sampler2D lightNormalSampler;
uniform sampler2D lightDepthSampler;

uniform mat4 projectionMatrix;
uniform mat4 projectionInverseMatrix;
uniform mat4 biasedLightViewProjectionMatrix;
uniform mat4 biasedLightViewProjectionInverseMatrix;
uniform vec3 worldLightPos;
uniform vec3 cameraEye;
uniform mat4 viewMatrix;
uniform mat4 viewInvertedMatrix;
uniform float zFar;
uniform float zNear;
uniform vec2 screenSize;

const float ambientFactor = 0.25;
const float specularFactor = 0.75;


void main()
{
    float projectionA = projectionMatrix[3][2];
    float projectionB = zFar / (zFar - zNear);
    float d = linearDepth(depthSampler, v_uv, projectionA, projectionB);
    vec3 fragNormal = texture(faceNormalSampler, v_uv, 0).xyz * 2.0 - 1.0;

    vec3 fragViewCoord = d * v_viewRay / zFar / 2.0; // unclear why the 2.0 is necessary
    vec3 fragWorldCoord = (viewInvertedMatrix * vec4(fragViewCoord, 1.0)).xyz;

    vec3 L = normalize(worldLightPos - fragWorldCoord);
    vec3 V = normalize(cameraEye - fragWorldCoord);
    vec3 H = normalize(L + V);
    float ndotl = dot(fragNormal, L);
    float ndotH = dot(fragNormal, H);

    vec4 scoord = biasedLightViewProjectionMatrix * vec4(fragWorldCoord, 1.0);


    //float shadowFactor = shadowmapComparisonVSM(shadowmap, scoord.xy/scoord.w, fragWorldCoord, worldLightPos);
    //shadowFactor *= step(0.0, sign(scoord.w));

    scoord.xyz /= scoord.w;
    vec3 lightColor = texture(lightDiffuseSampler, scoord.xy).rgb;
    vec3 lightNormal = texture(lightNormalSampler, scoord.xy).rgb * 2.0 - 1.0;
    float lightDepth = texture(lightDepthSampler, scoord.xy).r;

    //vec4 fragWorldCoords = biasedLightViewProjectionInverseMatrix * vec4(scoord.xy, lightDepth, 1.0);
    vec3 acc = vec3(0.0);

    const ivec2 iterations = ivec2(40, 10);
    int row = 3;
    for (int x = 2; x < iterations.x-2; x++) {
        for (int y = 0; y < iterations.y; y++) {
            vec2 texcoords = (vec2(x, y) + 0.5) / iterations;
            float depth = texture(lightDepthSampler, texcoords).r;
            vec4 vplWorldcoords = biasedLightViewProjectionInverseMatrix * vec4(texcoords, depth, 1.0);
            vplWorldcoords.xyz /= vplWorldcoords.w;

            vec3 vplNormal = texture(lightNormalSampler, texcoords).xyz * 2.0 - 1.0;
            //vplWorldcoords.xyz += vplNormal * 1;

            vec3 diff = fragWorldCoord - vplWorldcoords.xyz ;
        	float dist = length(diff);
            vec3 normalizedDiff = diff / dist;

            vec3 vplColor = texture(lightDiffuseSampler, texcoords).xyz;

            float isNearLight = 1.0 - step(15.0, dist);


            vec3 debugSplotch = isNearLight * vplColor / dist * 1000.0;

            float attenuation = 1.0 / (pow(dist / 5000.0, 4.0) * 10000);
            attenuation = min(attenuation, 100.0);

            //acc += debugSplotch;
            acc += vplColor * max(0.0, dot(vplNormal, normalizedDiff)) * max(0.0, dot(fragNormal, -normalizedDiff)) * attenuation ;

    	   //check output of that last line
        }
    }

    outColor = vec3(acc/100.0);
}
