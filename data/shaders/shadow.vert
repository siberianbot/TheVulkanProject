#version 450

layout (push_constant) uniform MeshConstants {
    mat4 matrix;
} meshConstants;

layout (location = 0) in vec3 inPosition;

void main() {
    gl_Position = meshConstants.matrix * vec4(inPosition, 1.0);
}