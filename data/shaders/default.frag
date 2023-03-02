#version 450

layout (set = 1, binding = 0) uniform sampler2DArray textureSampler;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inUV;

layout (location = 0) out vec4 outColor;

void main() {
    outColor = texture(textureSampler, vec3(inUV, 0));
}
