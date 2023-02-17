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

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct PhysicalDeviceInfo {
    std::optional<uint32_t> graphicsFamilyIdx;
    std::optional<uint32_t> presentFamilyIdx;
    std::vector<std::string> extensions;
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    std::vector<VkPresentModeKHR> presentModes;
    std::optional<VkFormat> depthFormat;
    VkSampleCountFlagBits msaaSamples;
    float maxSamplerAnisotropy;

    [[nodiscard]] bool isSuitable() const {
        bool requiredExtensionsSupported = std::all_of(VK_DEVICE_EXTENSIONS.begin(), VK_DEVICE_EXTENSIONS.end(),
                                                       [this](const char *requiredExtension) {
                                                           return std::find(extensions.begin(), extensions.end(),
                                                                            requiredExtension) != extensions.end();
                                                       });

        return graphicsFamilyIdx.has_value() &&
               presentFamilyIdx.has_value() &&
               requiredExtensionsSupported &&
               !surfaceFormats.empty() &&
               !presentModes.empty() &&
               depthFormat.has_value();
    }
};

struct UniformBufferObject {
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

class Engine;

class Renderer {
private:
    Engine *engine;

    VkInstance instance;

    VkExtent2D currentExtent;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice;
    PhysicalDeviceInfo physicalDeviceInfo;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkCommandPool commandPool;
    std::array<VkCommandBuffer, VK_MAX_INFLIGHT_FRAMES> commandBuffers;
    std::array<VkFence, VK_MAX_INFLIGHT_FRAMES> fences;
    std::array<VkSemaphore, VK_MAX_INFLIGHT_FRAMES> imageAvailableSemaphores, renderFinishedSemaphores;

    std::array<VkBuffer, VK_MAX_INFLIGHT_FRAMES> uniformBuffers;
    std::array<VkDeviceMemory, VK_MAX_INFLIGHT_FRAMES> uniformBufferMemory;
    std::array<void *, VK_MAX_INFLIGHT_FRAMES> uniformBufferMemoryMapped;
    VkSampler textureSampler;
    VkDescriptorPool descriptorPool;
    VkPipelineLayout pipelineLayout;
    VkDescriptorSetLayout descriptorSetLayout;

    VkSwapchainKHR swapchain;
    std::vector<VkImage> swapchainImages;
    VkFormat swapchainFormat;
    VkExtent2D swapchainExtent;
    std::vector<VkImageView> swapchainImageViews;

    VkImage colorImage, depthImage;
    VkImageView colorImageView, depthImageView;
    VkDeviceMemory colorImageMemory, depthImageMemory;

    std::vector<RenderpassBase*> _renderpasses;
    SceneRenderpass *_sceneRenderpass;

    uint32_t frameIdx = 0;
    bool resizeRequested = false;
    UniformBufferObject ubo = {};

    PhysicalDeviceInfo getPhysicalDeviceInfo(VkPhysicalDevice device);
    uint32_t getSuitableMemoryType(uint32_t memoryType, VkMemoryPropertyFlags memoryProperty);

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectMask);
    VkImage createImage(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage,
                        VkSampleCountFlagBits samples);
    VkBuffer createBuffer(VkDeviceSize size, VkBufferUsageFlags usage);

    VkDeviceMemory allocateMemoryForImage(VkImage image, VkMemoryPropertyFlags memoryProperty);
    VkDeviceMemory allocateMemoryForBuffer(VkBuffer buffer, VkMemoryPropertyFlags memoryProperty);

    void initInstance();

    void initSurface(GLFWwindow *window);

    void initPhysicalDevice();
    void initDevice();

    void initCommand();
    void initSync();

    void initUniformBuffers();
    void initTextureSampler();
    void initDescriptors();
    void initLayouts();

    void initSwapchain();
    void initSwapchainResources();

    void cleanupSwapchain();

    void handleResize();

    BufferData uploadVertices(const std::vector<Vertex> &vertices);
    BufferData uploadIndices(const std::vector<uint32_t> &indices);
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
