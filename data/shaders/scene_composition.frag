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
    vec3 position;
    float range;
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

float projectShadowMap(int shadowIdx, vec4 shadowCoord)
{
    vec4 normalized = shadowCoord / shadowCoord.w;

    if (normalized.z > -1.0 && normalized.z < 1.0)
    {
        float dist = texture(shadowMaps, vec3(normalized.st, shadowIdx)).r;
        if (normalized.w > 0.0 && dist < normalized.z)
        {
            return scene.ambient;
        }
    }

    return 1.0f;
}

void main() {
    vec4 albedo = subpassLoad(albedo, 0);
    vec3 position = subpassLoad(position, 0).rgb;
    vec3 normal = subpassLoad(normal, 0).rgb;
    float specular = subpassLoad(specular, 0).r;

    vec3 fragColor = albedo.rgb;

    for (int lightIdx = 0; lightIdx < scene.lightCount; lightIdx++) {
        float dist = length(lights[lightIdx].position - position);

        vec3 L = normalize(lights[lightIdx].position - position);
        vec3 V = normalize(camera.position - position);

        float atten = lights[lightIdx].range / (pow(dist, 2.0) + 1.0);

        vec3 N = normalize(normal);
        float NdotL = max(0.0, dot(N, L));
        vec3 diff = lights[lightIdx].color * albedo.rgb * NdotL * atten;

        vec3 R = reflect(-L, N);
        float NdotR = max(0.0, dot(R, V));
        vec3 spec = lights[lightIdx].color * specular * pow(NdotR, 32.0) * atten;

        fragColor += diff + spec;
    }

    if (scene.lightCount > 0) {
        fragColor /= scene.lightCount;
    }

    float shadow = scene.ambient;

    for (int shadowIdx = 0; shadowIdx < scene.shadowCount; shadowIdx++) {
        float dist = length(lights[shadowIdx].position - position);
        float atten = lights[shadowIdx].range / (pow(dist, 2.0) + 1.0);

        vec4 inShadowCoord = (biasMat * shadows[shadowIdx].matrix) * vec4(position, 1);
        shadow += atten * projectShadowMap(shadowIdx, inShadowCoord);
    }

    outColor = fragColor;

    // TODO:
    //    int numLights = min(scene.numLights, MAX_NUM_LIGHTS);
    //
    //    vec3 fragColor = AMBIENT * albedo.rgb;
    //    float shadow = numLights > 0 ? 0.0f : 1.0f;
    //
    //    for (int idx = 0; idx < numLights; idx++)
    //    {

    //
    //        fragColor += diff + spec;
    //
    //    }
    //
    //    if (numLights > 0) {
    //        shadow /= numLights;
    //    }
    //
    //    outColor = vec4(shadow * fragColor, albedo.a);
}
