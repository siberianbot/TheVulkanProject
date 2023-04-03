project('TheVulkanProject', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++20'])

add_project_arguments('-DGLM_FORCE_DEPTH_ZERO_TO_ONE', language: 'cpp')

# turn on VSync (see PhysicalDevice::getPreferredPresentMode)
# add_project_arguments('-DVSYNC', language: 'cpp')

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

    # Rendering
    'src/Rendering/RenderingManager.cpp',
    'src/Rendering/CommandExecution.cpp',
    'src/Rendering/CommandExecutor.cpp',
    'src/Rendering/PhysicalDevice.cpp',
    'src/Rendering/Renderer.cpp',
    'src/Rendering/RenderingDevice.cpp',
    'src/Rendering/RenderingLayoutsManager.cpp',
    'src/Rendering/RenderingObjectsAllocator.cpp',
    'src/Rendering/Swapchain.cpp',
    'src/Rendering/VulkanObjectsAllocator.cpp',

    'src/Rendering/Builders/AttachmentBuilder.cpp',
    'src/Rendering/Builders/BufferObjectBuilder.cpp',
    'src/Rendering/Builders/DescriptorPoolBuilder.cpp',
    'src/Rendering/Builders/DescriptorSetLayoutBuilder.cpp',
    'src/Rendering/Builders/FramebufferBuilder.cpp',
    'src/Rendering/Builders/ImageObjectBuilder.cpp',
    'src/Rendering/Builders/ImageViewObjectBuilder.cpp',
    'src/Rendering/Builders/PipelineBuilder.cpp',
    'src/Rendering/Builders/PipelineLayoutBuilder.cpp',
    'src/Rendering/Builders/RenderpassBuilder.cpp',
    'src/Rendering/Builders/SamplerBuilder.cpp',
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
    'src/Rendering/Renderpasses/DebugUIRenderpass.cpp',
    'src/Rendering/Renderpasses/SceneRenderpass.cpp',
    'src/Rendering/Renderpasses/ShadowRenderpass.cpp',

    'src/Rendering/Stages/DebugUIStage.cpp',
    'src/Rendering/Stages/SceneStage.cpp',

    # Resources
    'src/Resources/Resource.cpp',
    'src/Resources/ResourceManager.cpp',
    'src/Resources/MeshResource.cpp',
    'src/Resources/ImageBaseResource.cpp',
    'src/Resources/ImageResource.cpp',
    'src/Resources/CubeImageResource.cpp',
    'src/Resources/ShaderResource.cpp',

    # Objects
    'src/Objects/Object.cpp',
    'src/Objects/Camera.cpp',
    'src/Objects/LightSource.cpp',
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

    # Types
    'src/Types/LightSourceType.cpp',
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
