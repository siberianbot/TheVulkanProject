project('TheVulkanProject', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++20'])

# turn on VSync (see PhysicalDevice::getPreferredPresentMode)
# add_project_arguments('-DVSYNC', language: 'cpp')

vulkan = dependency('vulkan')
glfw = dependency('glfw3')
imgui = subproject('imgui', default_options: ['default_library=static'])

src = [
    'src/Main.cpp',
    'src/Engine.cpp',
    'src/Input.cpp',
    'src/Camera.cpp',

    # Rendering
    'src/Rendering/Renderer.cpp',
    'src/Rendering/PhysicalDevice.cpp',
    'src/Rendering/RenderingDevice.cpp',
    'src/Rendering/CommandExecutor.cpp',
    'src/Rendering/Swapchain.cpp',
    'src/Rendering/RenderingResourcesManager.cpp',

    'src/Rendering/FramebuffersBuilder.cpp',
    'src/Rendering/PipelineBuilder.cpp',
    'src/Rendering/RenderpassBuilder.cpp',

    'src/Rendering/RenderingObjectsFactory.cpp',
    'src/Rendering/Objects/BufferObject.cpp',
    'src/Rendering/Objects/DescriptorSetObject.cpp',
    'src/Rendering/Objects/FenceObject.cpp',
    'src/Rendering/Objects/ImageObject.cpp',
    'src/Rendering/Objects/ImageViewObject.cpp',
    'src/Rendering/Objects/RenderingLayoutObject.cpp',
    'src/Rendering/Objects/SemaphoreObject.cpp',

    'src/Rendering/Renderpasses/RenderpassBase.cpp',
    'src/Rendering/Renderpasses/ClearRenderpass.cpp',
    'src/Rendering/Renderpasses/FinalRenderpass.cpp',
    'src/Rendering/Renderpasses/ImguiRenderpass.cpp',
    'src/Rendering/Renderpasses/SceneRenderpass.cpp',
    'src/Rendering/Renderpasses/SkyboxRenderpass.cpp',

    # Resources
    'src/Resources/Mesh.cpp',
    'src/Resources/Texture.cpp',

    # Scene
    'src/Scene/Object.cpp',
    'src/Scene/Scene.cpp',

    # Debug only
    'src/DebugUI.cpp',
]

# shaders compilation
shaders = [
    'data/shaders/default.frag',
    'data/shaders/default.vert'
]

foreach shader : shaders
    run_command('glslangValidator', '-V', shader, '-o', shader + '.spv', check: true)
endforeach

exe = executable('thevulkanproject', src,
  dependencies: [ glfw, vulkan, imgui.get_variable('imgui_dep') ])
