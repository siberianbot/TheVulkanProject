project('TheVulkanProject', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++20'])

add_project_arguments('-DGLM_FORCE_DEPTH_ZERO_TO_ONE', language: 'cpp')

deps = [
    dependency('vulkan'),
    subproject('glfw').get_variable('glfw_dep'),
    subproject('glm').get_variable('glm_dep'),
    subproject('imgui', default_options: ['default_library=static']).get_variable('imgui_dep'),
    subproject('nlohmann_json').get_variable('nlohmann_json_dep'),
    subproject('fmt').get_variable('fmt_dep')
    # TODO: https://github.com/nothings/stb
    # TODO: https://github.com/tinyobjloader/tinyobjloader
]

src = [
    'src/Main.cpp',

    # Engine
    'src/Engine/Engine.cpp',
    'src/Engine/EngineError.cpp',
    'src/Engine/Log.cpp',
    'src/Engine/VarCollection.cpp',
    'src/Engine/InputProcessor.cpp',

    # Events
    'src/Events/Event.cpp',
    'src/Events/EventQueue.cpp',

    # Rendering System
    'src/Rendering/CommandManager.cpp',
    'src/Rendering/GpuAllocator.cpp',
    'src/Rendering/GpuManager.cpp',
    'src/Rendering/GpuResourceManager.cpp',
    'src/Rendering/Renderer.cpp',
    'src/Rendering/SurfaceManager.cpp',
    'src/Rendering/Swapchain.cpp',
    'src/Rendering/SwapchainManager.cpp',
    'src/Rendering/Graph/RenderGraph.cpp',
    'src/Rendering/Graph/RenderGraphBuilder.cpp',
    'src/Rendering/Graph/RenderSubgraph.cpp',
    'src/Rendering/Proxies/CommandBufferProxy.cpp',
    'src/Rendering/Proxies/LogicalDeviceProxy.cpp',
    'src/Rendering/Proxies/PhysicalDeviceProxy.cpp',

    # Resources
    'src/Resources/Resource.cpp',
    'src/Resources/ResourceData.cpp',
    'src/Resources/ResourceDatabase.cpp',
    'src/Resources/ResourceLoader.cpp',
    'src/Resources/Readers/ImageReader.cpp',
    'src/Resources/Readers/MeshReader.cpp',
    'src/Resources/Readers/SceneReader.cpp',

    # Objects
    'src/Objects/Object.cpp',
    'src/Objects/Camera.cpp',
    'src/Objects/LightSource.cpp',
    'src/Objects/LightSourceType.cpp',
    'src/Objects/Prop.cpp',
    'src/Objects/World.cpp',
    'src/Objects/Components/Component.cpp',
    'src/Objects/Components/ModelComponent.cpp',
    'src/Objects/Components/PositionComponent.cpp',
    'src/Objects/Components/SkyboxComponent.cpp',

    # Scene
    'src/Scene/Scene.cpp',
    'src/Scene/SceneNode.cpp',
    'src/Scene/SceneIterator.cpp',
    'src/Scene/SceneManager.cpp',

    # Debug
    'src/Debug/DebugUIRoot.cpp',
    'src/Debug/UI/LogWindow.cpp',
    'src/Debug/UI/MainMenuBar.cpp',
    'src/Debug/UI/ObjectEditorWindow.cpp',
    'src/Debug/UI/ObjectEditVisitor.cpp',
    'src/Debug/UI/ResourcesListWindow.cpp',
    'src/Debug/UI/SceneTreeWindow.cpp',
    'src/Debug/UI/ShaderCodeEditorWindow.cpp',
    'src/Debug/UI/VariablesWindow.cpp',

    # System
    'src/System/Window.cpp',
]

# shaders compilation
shaders = [
    'data/shaders/passthrough.vert',
    'data/shaders/scene-composition.frag',
    'data/shaders/scene-model.frag',
    'data/shaders/scene-model.vert',
    'data/shaders/shadow.frag',
    'data/shaders/shadow.vert',
    'data/shaders/skybox.frag'
]

foreach shader : shaders
    run_command('glslangValidator', '-gVS', '-V', shader, '-o', shader + '.spv', check: true)
endforeach

exe = executable('thevulkanproject', src, dependencies: deps)
