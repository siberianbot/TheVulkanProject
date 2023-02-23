#ifndef RENDERING_COMMON_HPP
#define RENDERING_COMMON_HPP

#include <cstdint>

#include <vulkan/vulkan.hpp>

static constexpr std::array<const char *, 1> VALIDATION_LAYERS = {
        "VK_LAYER_KHRONOS_validation"
};

static constexpr std::array<const char *, 1> DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static constexpr const uint32_t MAX_INFLIGHT_FRAMES = 2;

static constexpr const uint32_t MAX_ATTACHMENT_COUNT = 3; // TODO: ?
static constexpr const uint32_t COLOR_ATTACHMENT_IDX = 0;
static constexpr const uint32_t DEPTH_ATTACHMENT_IDX = 1;
static constexpr const uint32_t RESOLVE_ATTACHMENT_IDX = 2;

static constexpr const uint32_t MAX_MESH_DATA_DESCRIPTOR_SETS = 1024 * MAX_INFLIGHT_FRAMES;

static constexpr const char *DEFAULT_VERTEX_SHADER = "data/shaders/default.vert.spv";
static constexpr const char *DEFAULT_FRAGMENT_SHADER = "data/shaders/default.frag.spv";

static constexpr void vkEnsure(VkResult vkExpression) {
    if (vkExpression != VK_SUCCESS) {
        throw std::runtime_error("Vulkan runtime error");
    }
}

#endif // RENDERING_COMMON_HPP
