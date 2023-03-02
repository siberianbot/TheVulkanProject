#version 450

layout (set = 1, binding = 0) uniform samplerCube textureSampler;

layout (location = 0) in vec3 inPosition;

layout (location = 0) out vec4 outColor;

void main() {
    outColor = texture(textureSampler, inPosition);
}
