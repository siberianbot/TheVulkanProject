#version 450

layout (input_attachment_index = 0, binding = 0) uniform subpassInputMS inputSkybox;
layout (input_attachment_index = 1, binding = 1) uniform subpassInputMS inputAlbedo;
layout (input_attachment_index = 2, binding = 2) uniform subpassInputMS inputPosition;
layout (input_attachment_index = 3, binding = 3) uniform subpassInputMS inputNormal;
layout (input_attachment_index = 4, binding = 4) uniform subpassInputMS inputSpecular;

layout (location = 0) out vec3 outColor;

vec3 blend(vec4 back, vec4 front) {
    return (1 - front.a) * back.rgb + front.a * front.rgb;
}

void main() {
    vec4 skybox = subpassLoad(inputSkybox, 0);
    vec4 albedo = subpassLoad(inputAlbedo, 0);

    outColor = blend(skybox, albedo);
}
