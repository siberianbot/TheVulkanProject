#version 450

layout (binding = 1) uniform sampler2D textureSampler;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inTexCoord;

layout (location = 0) out vec4 outColor;

void main() {
    outColor = texture(textureSampler, inTexCoord);
}
