// TODO: improve shadowing/lighting

#version 450

layout (constant_id = 0) const uint SHADOW_COUNT = 32;
layout (constant_id = 1) const uint LIGHT_COUNT = 128;

layout (binding = 0, input_attachment_index = 0) uniform subpassInputMS albedo;
layout (binding = 1, input_attachment_index = 1) uniform subpassInputMS position;
layout (binding = 2, input_attachment_index = 2) uniform subpassInputMS normal;
layout (binding = 3, input_attachment_index = 3) uniform subpassInputMS specular;

layout (binding = 4) uniform sampler2DArray shadowMaps;

layout (binding = 5) uniform ShadowData {
    mat4 matrix;
} shadows[SHADOW_COUNT];

layout (binding = 6) uniform LightData {
    mat4 matrix;
    vec3 position;
    vec3 color;
    float range;
} lights[LIGHT_COUNT];

layout (binding = 7) uniform CameraData {
    vec3 position;
} camera;

layout (binding = 8) uniform SceneData {
    float ambient;
    uint shadowCount;
    uint lightCount;
} scene;

layout (location = 0) out vec3 outColor;

const mat4 biasMat = mat4(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.5, 0.5, 0.0, 1.0
);

//float textureProj(int lightIdx, vec4 shadowCoord)
//{
//    vec4 normalized = shadowCoord / shadowCoord.w;
//
//    if (normalized.z > -1.0 && normalized.z < 1.0)
//    {
//        float dist = texture(sceneShadows[lightIdx], normalized.st).r;
//        if (normalized.w > 0.0 && dist < normalized.z)
//        {
//            return AMBIENT;
//        }
//    }
//
//    return 1.0f;
//}

void main() {
    vec4 albedo = subpassLoad(albedo, 0);
    vec3 position = subpassLoad(position, 0).rgb;
    vec3 normal = subpassLoad(normal, 0).rgb;
    float specular = subpassLoad(specular, 0).r;

    outColor = albedo.rgb;

    // TODO:
//    int numLights = min(scene.numLights, MAX_NUM_LIGHTS);
//
//    vec3 fragColor = AMBIENT * albedo.rgb;
//    float shadow = numLights > 0 ? 0.0f : 1.0f;
//
//    for (int idx = 0; idx < numLights; idx++)
//    {
//        vec3 L = scene.lights[idx].position - position;
//        float dist = length(L);
//
//        L = normalize(scene.lights[idx].position - position);
//        vec3 V = normalize(scene.cameraPosition - position);
//
//        float atten = scene.lights[idx].radius / (pow(dist, 2.0) + 1.0);
//
//        vec3 N = normalize(normal);
//        float NdotL = max(0.0, dot(N, L));
//        vec3 diff = scene.lights[idx].color * albedo.rgb * NdotL * atten;
//
//        vec3 R = reflect(-L, N);
//        float NdotR = max(0.0, dot(R, V));
//        vec3 spec = scene.lights[idx].color * specular * pow(NdotR, 32.0) * atten;
//
//        fragColor += diff + spec;
//
//        vec4 inShadowCoord = (biasMat * scene.lights[idx].projection) * vec4(position, 1);
//        shadow += atten * textureProj(idx, inShadowCoord);
//    }
//
//    if (numLights > 0) {
//        shadow /= numLights;
//    }
//
//    outColor = vec4(shadow * fragColor, albedo.a);
}
