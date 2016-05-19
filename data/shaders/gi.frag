#version 330

#extension GL_ARB_shading_language_include : require
#include </data/shaders/common/reprojection.glsl>

in vec2 v_uv;
in vec3 v_viewRay;

out vec3 outColor;

uniform sampler2D faceNormalSampler;
uniform sampler2D depthSampler;
uniform sampler2D lightDiffuseSampler;
uniform sampler2D lightNormalSampler;
uniform sampler2D lightDepthSampler;
uniform sampler2D ismDepthSampler;

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

bool debug = true;

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
    float projectionA = projectionMatrix[3][2];
    float projectionB = zFar / (zFar - zNear);
    float d = linearDepth(depthSampler, v_uv, projectionA, projectionB);
    vec3 fragNormal = texture(faceNormalSampler, v_uv).xyz * 2.0 - 1.0;

    vec3 fragViewCoord = d * v_viewRay / zFar / 2.0; // unclear why the 2.0 is necessary
    vec3 fragWorldCoord = (viewInvertedMatrix * vec4(fragViewCoord, 1.0)).xyz;

    vec3 acc = vec3(0.0);

    const ivec2 maxIterations = ivec2(16,16);
    ivec2 from = ivec2(0, 0);
    ivec2 to = ivec2(16, 16);
    //from = ivec2(7,8);
    //to = ivec2(9,9);
    int numIterations = (to.x - from.x) * (to.y - from.y);

    for (int x = from.x; x < to.x; x++) {
        for (int y = from.y; y < to.y; y++) {
            vec2 texcoords = (vec2(x, y)) / vec2(maxIterations);
            float depth = texture(lightDepthSampler, texcoords).r;
            vec4 vplWorldcoords = biasedLightViewProjectionInverseMatrix * vec4(texcoords, depth, 1.0);
            vplWorldcoords.xyz /= vplWorldcoords.w;

            vec3 vplNormal = texture(lightNormalSampler, texcoords).xyz * 2.0 - 1.0;

            vec3 diff = fragWorldCoord - vplWorldcoords.xyz ;
        	float dist = length(diff);
            vec3 normalizedDiff = diff / dist;


            vec3 vplColor = texture(lightDiffuseSampler, texcoords).xyz;

            // debug splotch
            float isNearLight = 1.0 - step(15.0, dist);
            vec3 debugSplotch = isNearLight * vplColor / dist / dist * 30.0;
            acc += debugSplotch;

            // angle and attenuation
            float angleFactor = max(0.0, dot(vplNormal, normalizedDiff)) * max(0.0, dot(fragNormal, -normalizedDiff));
            if (angleFactor <= 0.0)
                continue;

            float toMetersFactor = 0.01;
            float attenuation = 1.0 / pow(dist * toMetersFactor, 4.0);

            mat4 vpl_view = lookAtRH(vplWorldcoords.xyz, vplWorldcoords.xyz + vplNormal, vec3(0.0, 1.0, 0.01));

            vec4 v = vpl_view * vec4(fragWorldCoord, 1.0);

            // paraboloid projection
            v.xyz /= dist;
            v.z = 1.0 - v.z;
            v.xy /= v.z;
            v.z = dist / (2000.0);

            // scale and bias to texcoords
            v.xy += 1.0;
            v.xy /= 2.0;
            v.xy /= maxIterations;
            v.xy += vec2(x, y) / maxIterations;

            // ISM shadowing
            float occluderDepth = texture(ismDepthSampler, v.xy).x;
            float shadowValue = v.z - occluderDepth;
            shadowValue = smoothstep(0.90, 1.0, 1 - shadowValue);

            float arbitraryIntensityFactor = 10000.0;
            float arbitraryClampingValue = arbitraryIntensityFactor * 0.001;
            float factor =  angleFactor * attenuation * shadowValue * arbitraryIntensityFactor;
            factor = min(factor, arbitraryClampingValue);

            acc += vplColor * factor / numIterations;
        }
    }

    outColor = vec3(acc);
}
