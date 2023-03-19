#ifndef RENDERING_COMMON_HPP
#define RENDERING_COMMON_HPP

#include <cstdint>

#include <glm/vec3.hpp>
#include <vulkan/vulkan.hpp>

#include "Constants.hpp"

static constexpr std::array<const char *, 1> VALIDATION_LAYERS = {
        "VK_LAYER_KHRONOS_validation"
};

static constexpr std::array<const char *, 1> DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static constexpr const int MAX_NUM_LIGHTS = 32;
static constexpr std::array<glm::vec3, 6> POINT_LIGHT_DIRECTIONS = {
        glm::vec3(1, 0, 0),
        glm::vec3(-1, 0, 0),
        glm::vec3(0, 1, 0),
        glm::vec3(0, -1, 0),
        glm::vec3(0, 0, 1),
        glm::vec3(0, 0, -1)
};

static constexpr void vkEnsure(VkResult vkExpression) {
    if (vkExpression != VK_SUCCESS) {
        throw std::runtime_error("Vulkan runtime error");
    }
}

#endif // RENDERING_COMMON_HPP
