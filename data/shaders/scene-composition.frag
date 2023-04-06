// TODO: improve shadowing/lighting

#version 450

struct ShadowData {
    mat4 matrix;
    vec3 position;
    float range;
};

struct LightData {
    vec3 position;
    vec3 color;
    float range;
};

layout (constant_id = 0) const uint SHADOW_COUNT = 32;
layout (constant_id = 1) const uint LIGHT_COUNT = 128;

layout (binding = 0, input_attachment_index = 0) uniform subpassInputMS albedo;
layout (binding = 1, input_attachment_index = 1) uniform subpassInputMS position;
layout (binding = 2, input_attachment_index = 2) uniform subpassInputMS normal;
layout (binding = 3, input_attachment_index = 3) uniform subpassInputMS specular;

layout (binding = 4) uniform sampler2DArray shadowMaps;

layout (binding = 5) uniform ShadowDataArray {
    ShadowData[SHADOW_COUNT] data;
} shadows;

layout (binding = 6) uniform LightDataArray {
    LightData[LIGHT_COUNT] data;
} lights;

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
        float dist = length(lights.data[lightIdx].position - position);

        vec3 L = normalize(lights.data[lightIdx].position - position);
        vec3 V = normalize(camera.position - position);

        float atten = lights.data[lightIdx].range / (pow(dist, 2.0) + 1.0);

        vec3 N = normalize(normal);
        float NdotL = max(0.0, dot(N, L));
        vec3 diff = lights.data[lightIdx].color * albedo.rgb * NdotL * atten;

        vec3 R = reflect(-L, N);
        float NdotR = max(0.0, dot(R, V));
        vec3 spec = lights.data[lightIdx].color * specular * pow(NdotR, 32.0) * atten;

        fragColor += diff + spec;
    }

    if (scene.lightCount > 0) {
        fragColor /= scene.lightCount;
    }

    float shadow = scene.ambient;

    for (int shadowIdx = 0; shadowIdx < scene.shadowCount; shadowIdx++) {
        float dist = length(shadows.data[shadowIdx].position - position);
        float atten = shadows.data[shadowIdx].range / (pow(dist, 2.0) + 1.0);

        vec4 inShadowCoord = (biasMat * shadows.data[shadowIdx].matrix) * vec4(position, 1);
        shadow += atten * projectShadowMap(shadowIdx, inShadowCoord);
    }

    if (scene.shadowCount > 0) {
        shadow /= scene.shadowCount;
    }

    outColor = shadow * fragColor;
}
