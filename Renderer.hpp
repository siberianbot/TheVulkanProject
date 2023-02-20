#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <optional>
#include <vector>
#include <string>
#include <algorithm>

#include "Mesh.hpp"
#include "Texture.hpp"
#include "Rendering/Renderpasses/RenderpassBase.hpp"
#include "RendererTypes.hpp"
#include "Rendering/Renderpasses/SceneRenderpass.hpp"
#include "Rendering/VulkanCommandExecutor.hpp"
#include "Rendering/VulkanPhysicalDevice.hpp"
#include "Rendering/RenderingObjectsFactory.hpp"
#include "Rendering/Swapchain.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct UniformBufferObject {
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

class Engine;

class Renderer {
private:
    Engine *engine;

    VkInstance instance;
    VkSurfaceKHR surface;

    VulkanPhysicalDevice *_physicalDevice;
    RenderingDevice *_renderingDevice;
    RenderingObjectsFactory *_renderingObjectsFactory;
    VulkanCommandExecutor *_commandExecutor;
    Swapchain *_swapchain;
    std::array<FenceObject *, VK_MAX_INFLIGHT_FRAMES> _fences;

    std::array<VkSemaphore, VK_MAX_INFLIGHT_FRAMES> imageAvailableSemaphores, renderFinishedSemaphores;

    std::array<BufferObject *, VK_MAX_INFLIGHT_FRAMES> uniformBuffers;
    VkSampler textureSampler;
    VkDescriptorPool descriptorPool;
    VkPipelineLayout pipelineLayout;
    VkDescriptorSetLayout descriptorSetLayout;

    std::vector<RenderpassBase *> _renderpasses;
    SceneRenderpass *_sceneRenderpass;

    uint32_t _currentFrameIdx = 0;
    bool resizeRequested = false;
    UniformBufferObject ubo = {};

    void initInstance();
    void initSurface(GLFWwindow *window);

    void initSync();

    void initUniformBuffers();
    void initTextureSampler();
    void initDescriptors();
    void initLayouts();

    void handleResize();

    BufferObject *uploadVertices(const std::vector<Vertex> &vertices);
    BufferObject *uploadIndices(const std::vector<uint32_t> &indices);
    ImageObject *uploadTexture(const std::string &texturePath);
    std::array<VkDescriptorSet, VK_MAX_INFLIGHT_FRAMES> initDescriptorSets(VkImageView textureImageView);

public:
    Renderer(Engine* engine);

    void init();
    void cleanup();

    void render();

    void requestResize(uint32_t width, uint32_t height);

    BoundMeshInfo *uploadMesh(const Mesh &mesh, const Texture &texture);
    void freeMesh(BoundMeshInfo *meshInfo);
};

#endif // RENDERER_HPP
