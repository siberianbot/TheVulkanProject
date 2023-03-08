#version 450

#define AMBIENT 0.15

layout (input_attachment_index = 0, binding = 0) uniform subpassInputMS inputSkybox;
layout (input_attachment_index = 1, binding = 1) uniform subpassInputMS inputAlbedo;
layout (input_attachment_index = 2, binding = 2) uniform subpassInputMS inputPosition;
layout (input_attachment_index = 3, binding = 3) uniform subpassInputMS inputNormal;
layout (input_attachment_index = 4, binding = 4) uniform subpassInputMS inputSpecular;

layout (constant_id = 0) const int MAX_NUM_LIGHTS = 32;

struct LightData {
    mat4 projection;
    vec3 position;
    vec3 color;
    float radius;
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
const float bias = 0.005;

float textureProj(int shadowIdx, vec4 shadowCoord)
{
    float shadow = 1.0;
    if (shadowCoord.z > -1.0 && shadowCoord.z < 1.0)
    {
        float dist = texture(sceneShadows[shadowIdx], shadowCoord.st).r;
        if (shadowCoord.w > 0.0 && dist < shadowCoord.z - bias)
        {
            shadow = AMBIENT;
        }
    }
    return shadow;
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

    vec3 fragColor = AMBIENT * albedo.rgb;

    int numLights = min(scene.numLights, MAX_NUM_LIGHTS);
    for (int idx = 0; idx < numLights; idx++)
    {
        vec3 L = scene.lights[idx].position - position;
        float dist = length(L);
        L = normalize(L);

        vec3 V = scene.cameraPosition - position;
        V = normalize(V);

        float atten = scene.lights[idx].radius / (pow(dist, 2.0) + 1.0);

        vec3 N = normalize(normal);
        float NdotL = max(0.0, dot(N, L));
        vec3 diff = scene.lights[idx].color * albedo.rgb * NdotL * atten;

        vec3 R = reflect(-L, N);
        float NdotR = max(0.0, dot(R, V));
        vec3 spec = scene.lights[idx].color * specular * pow(NdotR, 32.0) * atten;

        vec4 inShadowCoord = (biasMat * scene.lights[idx].projection) * vec4(position, 1);
        float shadow = textureProj(idx, inShadowCoord / inShadowCoord.w);

        fragColor += shadow * (diff + spec);
    }

    outColor = blend(skybox, vec4(fragColor, albedo.a));
}
