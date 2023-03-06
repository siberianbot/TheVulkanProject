#version 450

layout (input_attachment_index = 0, binding = 0) uniform subpassInputMS inputSkybox;
layout (input_attachment_index = 1, binding = 1) uniform subpassInputMS inputAlbedo;
layout (input_attachment_index = 2, binding = 2) uniform subpassInputMS inputPosition;
layout (input_attachment_index = 3, binding = 3) uniform subpassInputMS inputNormal;
layout (input_attachment_index = 4, binding = 4) uniform subpassInputMS inputSpecular;

layout (set = 1, binding = 0) uniform SceneData {
    vec3 cameraPosition;
} sceneData;

layout (location = 0) out vec3 outColor;

vec3 blend(vec4 back, vec4 front) {
    return (1 - front.a) * back.rgb + front.a * front.rgb;
}

void main() {
    #define AMBIENT 0.15
    vec3 LIGHT_POS = vec3(2, 1, 0);
    vec3 LIGHT_COLOR = vec3(1);
    float LIGHT_RADIUS = 10;

    vec4 skybox = subpassLoad(inputSkybox, 0);
    vec4 albedo = subpassLoad(inputAlbedo, 0);
    vec3 position = subpassLoad(inputPosition, 0).rgb;
    vec3 normal = subpassLoad(inputNormal, 0).rgb;
    float specular = subpassLoad(inputSpecular, 0).r;

    vec3 fragColor = AMBIENT * albedo.rgb;

    {
        vec3 L = LIGHT_POS - position;
        float dist = length(L);
        L = normalize(L);

        vec3 V = sceneData.cameraPosition - position;
        V = normalize(V);

        float atten = LIGHT_RADIUS / (pow(dist, 2.0) + 1.0);

        vec3 N = normalize(normal);
        float NdotL = max(0.0, dot(N, L));
        vec3 diff = LIGHT_COLOR * albedo.rgb * NdotL * atten;

        vec3 R = reflect(-L, N);
        float NdotR = max(0.0, dot(R, V));
        vec3 spec = LIGHT_COLOR * specular * pow(NdotR, 32.0) * atten;

        fragColor += diff + spec;
    }

    outColor = blend(skybox, vec4(fragColor, albedo.a));
}
