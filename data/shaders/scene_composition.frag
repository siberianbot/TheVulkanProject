// TODO: improve shadowing/lighting

#version 450

#define AMBIENT 0.15

layout (input_attachment_index = 0, binding = 0) uniform subpassInputMS inputSkybox;
layout (input_attachment_index = 1, binding = 1) uniform subpassInputMS inputAlbedo;
layout (input_attachment_index = 2, binding = 2) uniform subpassInputMS inputPosition;
layout (input_attachment_index = 3, binding = 3) uniform subpassInputMS inputNormal;
layout (input_attachment_index = 4, binding = 4) uniform subpassInputMS inputSpecular;

layout (constant_id = 0) const int MAX_NUM_LIGHTS = 32;

const int POINT_LIGHT = 0;
const int SPOT_LIGHT = 1;
const int RECT_LIGHT = 2;

struct LightData {
    mat4 projection;
    vec3 position;
    vec3 color;
    float radius;
    int kind;
};

layout (set = 1, binding = 0) uniform SceneData {
    vec3 cameraPosition;
    int numLights;
    LightData[MAX_NUM_LIGHTS] lights;
} scene;

layout (set = 1, binding = 1) uniform sampler2D sceneShadows[MAX_NUM_LIGHTS];

layout (location = 0) out vec3 outColor;

const mat4 biasMat = mat4(
0.5, 0.0, 0.0, 0.0,
0.0, 0.5, 0.0, 0.0,
0.0, 0.0, 1.0, 0.0,
0.5, 0.5, 0.0, 1.0);

float textureProj(int lightIdx, vec4 shadowCoord)
{
    vec4 normalized = shadowCoord / shadowCoord.w;

    if (scene.lights[lightIdx].kind != POINT_LIGHT) {
        if (shadowCoord.x < 0 || shadowCoord.y < 0) {
            return AMBIENT;
        }

        if (normalized.x < 0 || normalized.x > 1 || normalized.y < 0 || normalized.y > 1) {
            return AMBIENT;
        }
    }

    if (normalized.z > -1.0 && normalized.z < 1.0)
    {
        float dist = texture(sceneShadows[lightIdx], normalized.st).r;
        if (normalized.w > 0.0 && dist < normalized.z)
        {
            return AMBIENT;
        }
    }

    return 1.0f;
}

vec3 blend(vec4 back, vec4 front) {
    return (1 - front.a) * back.rgb + front.a * front.rgb;
}

void main() {
    vec4 skybox = subpassLoad(inputSkybox, 0);
    vec4 albedo = subpassLoad(inputAlbedo, 0);
    vec3 position = subpassLoad(inputPosition, 0).rgb;
    vec3 normal = subpassLoad(inputNormal, 0).rgb;
    float specular = subpassLoad(inputSpecular, 0).r;

    int numLights = min(scene.numLights, MAX_NUM_LIGHTS);

    vec3 fragColor = AMBIENT * albedo.rgb;
    float shadow = numLights > 0 ? 0.0f : 1.0f;

    for (int idx = 0; idx < numLights; idx++)
    {
        vec3 L = scene.lights[idx].position - position;
        float dist = length(L);

        L = normalize(scene.lights[idx].position - position);
        vec3 V = normalize(scene.cameraPosition - position);

        float atten = scene.lights[idx].radius / (pow(dist, 2.0) + 1.0);

        vec3 N = normalize(normal);
        float NdotL = max(0.0, dot(N, L));
        vec3 diff = scene.lights[idx].color * albedo.rgb * NdotL * atten;

        vec3 R = reflect(-L, N);
        float NdotR = max(0.0, dot(R, V));
        vec3 spec = scene.lights[idx].color * specular * pow(NdotR, 32.0) * atten;

        fragColor += diff + spec;

        vec4 inShadowCoord = (biasMat * scene.lights[idx].projection) * vec4(position, 1);
        shadow += atten * textureProj(idx, inShadowCoord);
    }

    if (numLights > 0) {
        shadow /= numLights;
    }

    outColor = blend(skybox, vec4(shadow * fragColor, albedo.a));
}
