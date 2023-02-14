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

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

static constexpr const int VK_MAX_INFLIGHT_FRAMES = 2;

static constexpr std::array<const char *, 1> VK_VALIDATION_LAYERS = {
        "VK_LAYER_KHRONOS_validation"
};

static constexpr std::array<const char *, 1> VK_DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

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

struct BufferData {
    VkBuffer buffer;
    VkDeviceMemory memory;
};

struct TextureData {
    VkImage image;
    VkImageView imageView;
    VkDeviceMemory memory;
};

struct BoundMeshInfo {
    uint32_t renderpassIdx;
    BufferData vertexBuffer;
    BufferData indexBuffer;
    TextureData texture;
    glm::mat4 model;
    uint32_t indicesCount;
    std::array<VkDescriptorSet, VK_MAX_INFLIGHT_FRAMES> descriptorSets;
};

struct UniformBufferObject {
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

struct Constants {
    alignas(16) glm::mat4 model;
};

struct Renderpass {
    VkRenderPass renderpass;
    std::array<VkClearValue, 2> clearValues;
    VkPipeline pipeline;
    std::vector<VkFramebuffer> framebuffers;
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

    std::array<Renderpass, 1> renderpasses;

    int currentFrame = 0;
    bool resizeRequested = false;
    std::vector<BoundMeshInfo *> meshes;
    UniformBufferObject ubo = {};

    PhysicalDeviceInfo getPhysicalDeviceInfo(VkPhysicalDevice device);
    uint32_t getSuitableMemoryType(uint32_t memoryType, VkMemoryPropertyFlags memoryProperty);

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectMask);
    VkImage createImage(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage,
                        VkSampleCountFlagBits samples);
    VkShaderModule createShaderModule(const std::string &path);
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

    VkRenderPass initRenderPass();
    VkPipeline initGraphicsPipeline(VkRenderPass renderpass);
    std::array<VkClearValue, 2> initClearColors();
    void initFramebuffers(Renderpass &renderpass);

    void cleanupSwapchain();

    void handleResize();

    Renderpass createRenderpass();
    void createRenderpassFramebuffers(Renderpass &renderpass);
    void destroyRenderpass(const Renderpass& renderpass);
    void destroyRenderpassFramebuffers(const Renderpass& renderpass);

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

    BoundMeshInfo *uploadMesh(uint32_t renderpassIdx, const Mesh &mesh, const Texture &texture);
    void freeMesh(BoundMeshInfo *meshInfo);
};

#endif // RENDERER_HPP
