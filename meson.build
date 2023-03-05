project('TheVulkanProject', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++20'])

add_project_arguments('-DGLM_FORCE_DEPTH_ZERO_TO_ONE', language: 'cpp')

# turn on VSync (see PhysicalDevice::getPreferredPresentMode)
# add_project_arguments('-DVSYNC', language: 'cpp')

vulkan = dependency('vulkan')
glfw = dependency('glfw3')
imgui = subproject('imgui', default_options: ['default_library=static'])

src = [
    'src/Main.cpp',
    'src/Engine.cpp',
    'src/Input.cpp',

    # Rendering
    'src/Rendering/Renderer.cpp',
    'src/Rendering/PhysicalDevice.cpp',
    'src/Rendering/RenderingDevice.cpp',
    'src/Rendering/CommandExecutor.cpp',
    'src/Rendering/Swapchain.cpp',
    'src/Rendering/RenderingResourcesManager.cpp',

    'src/Rendering/Builders/AttachmentBuilder.cpp',
    'src/Rendering/Builders/FramebufferBuilder.cpp',
    'src/Rendering/Builders/PipelineBuilder.cpp',
    'src/Rendering/Builders/RenderpassBuilder.cpp',
    'src/Rendering/Builders/SubpassBuilder.cpp',

    'src/Rendering/RenderingObjectsFactory.cpp',
    'src/Rendering/Objects/BufferObject.cpp',
    'src/Rendering/Objects/DescriptorSetObject.cpp',
    'src/Rendering/Objects/FenceObject.cpp',
    'src/Rendering/Objects/ImageObject.cpp',
    'src/Rendering/Objects/ImageViewObject.cpp',
    'src/Rendering/Objects/RenderingLayoutObject.cpp',
    'src/Rendering/Objects/SemaphoreObject.cpp',

    'src/Rendering/Renderpasses/RenderpassBase.cpp',
    'src/Rendering/Renderpasses/ImguiRenderpass.cpp',
    'src/Rendering/Renderpasses/SceneRenderpass.cpp',
    'src/Rendering/Renderpasses/SwapchainPresentRenderpass.cpp',

    # Resources
    'src/Resources/Mesh.cpp',
    'src/Resources/Texture.cpp',

    # Scene
    'src/Scene/Camera.cpp',
    'src/Scene/Object.cpp',
    'src/Scene/Scene.cpp',
    'src/Scene/Skybox.cpp',

    # Debug
    'src/Debug/DebugUI.cpp',
]

# shaders compilation
shaders = [
    'data/shaders/composition.frag',
    'data/shaders/composition.vert',
    'data/shaders/default.frag',
    'data/shaders/default.vert',
    'data/shaders/skybox.frag'
]

foreach shader : shaders
    run_command('glslangValidator', '-V', shader, '-o', shader + '.spv', check: true)
endforeach

exe = executable('thevulkanproject', src,
  dependencies: [ glfw, vulkan, imgui.get_variable('imgui_dep') ])
