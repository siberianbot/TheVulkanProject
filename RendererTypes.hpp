#ifndef RENDERERTYPES_HPP
#define RENDERERTYPES_HPP

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include "Rendering/Objects/BufferObject.hpp"
#include "Rendering/Objects/ImageObject.hpp"

static constexpr const int VK_MAX_INFLIGHT_FRAMES = 2;

static constexpr std::array<const char *, 1> VK_VALIDATION_LAYERS = {
        "VK_LAYER_KHRONOS_validation"
};

static constexpr std::array<const char *, 1> VK_DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct UniformBufferObject {
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

struct BoundMeshInfo {
    BufferObject *vertexBuffer;
    BufferObject *indexBuffer;
    ImageObject *texture;
    glm::mat4 model;
    uint32_t indicesCount;
    std::array<VkDescriptorSet, VK_MAX_INFLIGHT_FRAMES> descriptorSets;
};

struct Constants {
    alignas(16) glm::mat4 model;
};

#endif // RENDERERTYPES_HPP
