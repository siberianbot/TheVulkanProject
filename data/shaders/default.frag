#version 450

layout (set = 0, binding = 0) uniform sampler2DArray textureSampler;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inColor;
layout (location = 3) in vec2 inUV;

layout (location = 0) out vec4 outAlbedo;
layout (location = 1) out vec4 outPosition;
layout (location = 2) out vec4 outNormal;
layout (location = 3) out vec4 outSpecular;

void main() {
    outAlbedo = texture(textureSampler, vec3(inUV, 0));
    outPosition = vec4(inPosition, 1);
    outNormal = vec4(inNormal, 1);
    outSpecular = texture(textureSampler, vec3(inUV, 1));
}
