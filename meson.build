project('TheVulkanProject', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++20'])

add_project_arguments('-DGLM_FORCE_DEPTH_ZERO_TO_ONE', language: 'cpp')

# turn on VSync (see PhysicalDevice::getPreferredPresentMode)
# add_project_arguments('-DVSYNC', language: 'cpp')

deps = [
    dependency('vulkan'),
    dependency('glfw3'),
    subproject('imgui', default_options: ['default_library=static']).get_variable('imgui_dep'),
    subproject('nlohmann_json').get_variable('nlohmann_json_dep')
]

src = [
    'src/Main.cpp',

    # Engine
    'src/Engine/Engine.cpp',
    'src/Engine/EngineVars.cpp',
    'src/Engine/InputProcessor.cpp',

    # Events
    'src/Events/EventQueue.cpp',

    # Rendering
    'src/Rendering/Renderer.cpp',
    'src/Rendering/PhysicalDevice.cpp',
    'src/Rendering/RenderingDevice.cpp',
    'src/Rendering/CommandExecutor.cpp',
    'src/Rendering/Swapchain.cpp',
    'src/Rendering/RendererAllocator.cpp',

    'src/Rendering/Builders/AttachmentBuilder.cpp',
    'src/Rendering/Builders/DescriptorPoolBuilder.cpp',
    'src/Rendering/Builders/DescriptorSetLayoutBuilder.cpp',
    'src/Rendering/Builders/FramebufferBuilder.cpp',
    'src/Rendering/Builders/PipelineBuilder.cpp',
    'src/Rendering/Builders/PipelineLayoutBuilder.cpp',
    'src/Rendering/Builders/RenderpassBuilder.cpp',
    'src/Rendering/Builders/SpecializationInfoBuilder.cpp',
    'src/Rendering/Builders/SubpassBuilder.cpp',

    'src/Rendering/Objects/BufferObject.cpp',
    'src/Rendering/Objects/DescriptorSetObject.cpp',
    'src/Rendering/Objects/FenceObject.cpp',
    'src/Rendering/Objects/ImageObject.cpp',
    'src/Rendering/Objects/ImageViewObject.cpp',
    'src/Rendering/Objects/SemaphoreObject.cpp',
    'src/Rendering/Objects/ShaderObject.cpp',

    'src/Rendering/Renderpasses/RenderpassBase.cpp',
    'src/Rendering/Renderpasses/ImguiRenderpass.cpp',
    'src/Rendering/Renderpasses/SceneRenderpass.cpp',
    'src/Rendering/Renderpasses/ShadowRenderpass.cpp',
    'src/Rendering/Renderpasses/SwapchainPresentRenderpass.cpp',

    # Resources
    'src/Resources/Resource.cpp',
    'src/Resources/ResourceManager.cpp',
    'src/Resources/MeshResource.cpp',
    'src/Resources/ImageBaseResource.cpp',
    'src/Resources/ImageResource.cpp',
    'src/Resources/CubeImageResource.cpp',
    'src/Resources/ShaderResource.cpp',

    # Objects
    'src/Objects/Camera.cpp',
    'src/Objects/Light.cpp',
    'src/Objects/Object.cpp',
    'src/Objects/Skybox.cpp',
    'src/Objects/Data/RenderingData.cpp',

    # Scene
    'src/Scene/Scene.cpp',
    'src/Scene/SceneManager.cpp',

    # Debug
    'src/Debug/DebugUI.cpp',

    # Types
    'src/Types/ResourceType.cpp',

    # System
    'src/System/Window.cpp',
]

# shaders compilation
shaders = [
    'data/shaders/composition.frag',
    'data/shaders/composition.vert',
    'data/shaders/default.frag',
    'data/shaders/default.vert',
    'data/shaders/scene_composition.frag',
    'data/shaders/shadow.frag',
    'data/shaders/shadow.vert',
    'data/shaders/skybox.frag'
]

foreach shader : shaders
    run_command('glslangValidator', '-gVS', '-V', shader, '-o', shader + '.spv', check: true)
endforeach

exe = executable('thevulkanproject', src, dependencies: deps)
