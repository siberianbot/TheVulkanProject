#version 450

layout (push_constant) uniform MeshConstants {
    mat4 matrix;
    mat4 model;
} meshConstants;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inColor;
layout (location = 3) in vec2 inUV;

layout (location = 0) out vec3 outPosition;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outColor;
layout (location = 3) out vec2 outUV;

void main() {
    outPosition = (meshConstants.model * vec4(inPosition, 1)).xyz;
    outNormal = (meshConstants.model * vec4(inNormal, 1)).xyz;
    outColor = inColor;
    outUV = inUV;

    gl_Position = meshConstants.matrix * vec4(inPosition, 1.0);
}
