project('TheVulkanProject', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++20'])

vulkan = dependency('vulkan')
glfw = dependency('glfw3')

src = [
    'Main.cpp',
    'Engine.cpp',
    'Input.cpp',
    'Mesh.cpp',
    'Texture.cpp',
    'Camera.cpp',
    'Object.cpp',

    # Rendering
    'Rendering/Renderer.cpp',
    'Rendering/PhysicalDevice.cpp',
    'Rendering/RenderingDevice.cpp',
    'Rendering/CommandExecutor.cpp',
    'Rendering/Swapchain.cpp',
    'Rendering/RenderingResourcesManager.cpp',

    'Rendering/FramebuffersBuilder.cpp',
    'Rendering/PipelineBuilder.cpp',
    'Rendering/RenderpassBuilder.cpp',

    'Rendering/RenderingObjectsFactory.cpp',
    'Rendering/Objects/BufferObject.cpp',
    'Rendering/Objects/DescriptorSetObject.cpp',
    'Rendering/Objects/FenceObject.cpp',
    'Rendering/Objects/ImageObject.cpp',
    'Rendering/Objects/ImageViewObject.cpp',
    'Rendering/Objects/RenderingLayoutObject.cpp',
    'Rendering/Objects/SemaphoreObject.cpp',

    'Rendering/Renderpasses/RenderpassBase.cpp',
    'Rendering/Renderpasses/ClearRenderpass.cpp',
    'Rendering/Renderpasses/FinalRenderpass.cpp',
    'Rendering/Renderpasses/SceneRenderpass.cpp',
    'Rendering/Renderpasses/SkyboxRenderpass.cpp',
]

# shaders compilation
shaders = [
    'shaders/default.frag',
    'shaders/default.vert'
]

foreach shader : shaders
    run_command('glslangValidator', '-V', shader, '-o', shader + '.spv', check: true)
endforeach

exe = executable('thevulkanproject', src,  dependencies: [ glfw, vulkan ])
