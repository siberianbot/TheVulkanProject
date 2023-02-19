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
#include "RenderpassBase.hpp"
#include "RendererTypes.hpp"
#include "SceneRenderpass.hpp"
#include "Rendering/VulkanCommandExecutor.hpp"
#include "Rendering/VulkanPhysicalDevice.hpp"
#include "Rendering/RenderingObjectsFactory.hpp"

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

    VkExtent2D currentExtent;

    std::array<VkFence, VK_MAX_INFLIGHT_FRAMES> fences;
    std::array<VkSemaphore, VK_MAX_INFLIGHT_FRAMES> imageAvailableSemaphores, renderFinishedSemaphores;

    std::array<BufferObject *, VK_MAX_INFLIGHT_FRAMES> uniformBuffers;
    VkSampler textureSampler;
    VkDescriptorPool descriptorPool;
    VkPipelineLayout pipelineLayout;
    VkDescriptorSetLayout descriptorSetLayout;

    VkSwapchainKHR swapchain;
    std::vector<VkImage> swapchainImages;
    VkExtent2D swapchainExtent;
    std::vector<VkImageView> swapchainImageViews;

    VkImage colorImage, depthImage;
    VkImageView colorImageView, depthImageView;
    VkDeviceMemory colorImageMemory, depthImageMemory;

    std::vector<RenderpassBase *> _renderpasses;
    SceneRenderpass *_sceneRenderpass;

    uint32_t frameIdx = 0;
    bool resizeRequested = false;
    UniformBufferObject ubo = {};

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectMask);
    VkImage createImage(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage,
                        VkSampleCountFlagBits samples);

    VkDeviceMemory allocateMemoryForImage(VkImage image, VkMemoryPropertyFlags memoryProperty);

    void initInstance();
    void initSurface(GLFWwindow *window);

    void initSync();

    void initUniformBuffers();
    void initTextureSampler();
    void initDescriptors();
    void initLayouts();

    void initSwapchain();
    void initSwapchainResources();

    void cleanupSwapchain();

    void handleResize();

    BufferObject *uploadVertices(const std::vector<Vertex> &vertices);
    BufferObject *uploadIndices(const std::vector<uint32_t> &indices);
    TextureData uploadTexture(const std::string &texturePath);
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
