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
    'Camera.cpp'
]

exe = executable('thevulkanproject', src,  dependencies: [ glfw, vulkan ])
