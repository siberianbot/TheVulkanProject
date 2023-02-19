#ifndef VULKANCOMMON_HPP
#define VULKANCOMMON_HPP

#include <vulkan/vulkan.hpp>

static constexpr void vkEnsure(VkResult vkExpression) {
    if (vkExpression != VK_SUCCESS) {
        throw std::runtime_error("Vulkan runtime error");
    }
}

#endif // VULKANCOMMON_HPP
