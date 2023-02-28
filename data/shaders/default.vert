#version 450

layout (set = 0, binding = 0) uniform SceneData {
    mat4 view;
    mat4 projection;
} sceneData;

layout (push_constant) uniform MeshConstants {
    mat4 matrix;
} meshConstants;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inTexCoord;

layout (location = 0) out vec3 fragPosition;
layout (location = 1) out vec3 fragColor;
layout (location = 2) out vec2 fragTexCoord;

void main() {
    fragPosition = inPosition;
    fragColor = inColor;
    fragTexCoord = inTexCoord;

    gl_Position = meshConstants.matrix * vec4(inPosition, 1.0);
}
