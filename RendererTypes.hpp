#ifndef RENDERERTYPES_HPP
#define RENDERERTYPES_HPP

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include "Rendering/BufferObject.hpp"

static constexpr const int VK_MAX_INFLIGHT_FRAMES = 2;

static constexpr std::array<const char *, 1> VK_VALIDATION_LAYERS = {
        "VK_LAYER_KHRONOS_validation"
};

static constexpr std::array<const char *, 1> VK_DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct TextureData {
    VkImage image;
    VkImageView imageView;
    VkDeviceMemory memory;
};

struct BoundMeshInfo {
    BufferObject *vertexBuffer;
    BufferObject *indexBuffer;
    TextureData texture;
    glm::mat4 model;
    uint32_t indicesCount;
    std::array<VkDescriptorSet, VK_MAX_INFLIGHT_FRAMES> descriptorSets;
};

struct Constants {
    alignas(16) glm::mat4 model;
};

#endif // RENDERERTYPES_HPP
