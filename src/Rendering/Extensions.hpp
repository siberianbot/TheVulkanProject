#ifndef RENDERING_EXTENSIONS_HPP
#define RENDERING_EXTENSIONS_HPP

#include <array>

#include <vulkan/vulkan.hpp>

static constexpr const std::array<const char *, 1> REQUIRED_LAYERS = {
        "VK_LAYER_KHRONOS_validation"
};

static constexpr const std::array<const char *, 1> REQUIRED_INSTANCE_EXTENSIONS = {
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
};

static constexpr const std::array<const char *, 1> REQUIRED_DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#endif // RENDERING_EXTENSIONS_HPP
