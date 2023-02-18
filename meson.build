project('TheVulkanProject', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++20', 'std=c++20'])

vulkan = dependency('vulkan')
glfw = dependency('glfw3')

src = [
    'Main.cpp',
    'Engine.cpp',
    'Renderer.cpp',
    'Input.cpp',
    'Mesh.cpp',
    'Camera.cpp',
    'RenderpassBase.cpp',
    'ClearRenderpass.cpp',
    'FinalRenderpass.cpp',
    'SceneRenderpass.cpp',
    # 'SkyboxRenderpass.cpp',
    'VulkanRenderpassBuilder.cpp',
    'Rendering/VulkanFramebuffersBuilder.cpp',
    'Rendering/VulkanPipelineBuilder.cpp',
    'Rendering/VulkanCommandExecutor.cpp',
    'Rendering/VulkanPhysicalDevice.cpp'
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
