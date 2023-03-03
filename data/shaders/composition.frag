#version 450

layout (input_attachment_index = 0, binding = 0) uniform subpassInput inputScene;
layout (input_attachment_index = 1, binding = 1) uniform subpassInput inputImgui;

layout (location = 0) out vec3 outColor;

void main()
{
    vec4 scene = subpassLoad(inputScene);
    vec4 imgui = subpassLoad(inputImgui);

    outColor = (1 - imgui.a) * scene.rgb + imgui.a * imgui.rgb;
}