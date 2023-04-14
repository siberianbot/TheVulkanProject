#ifndef RENDERING_EXTENSIONS_HPP
#define RENDERING_EXTENSIONS_HPP

#include <algorithm>
#include <array>
#include <string_view>

#include <vulkan/vulkan.hpp>

static constexpr const std::array<std::string_view, 1> REQUIRED_LAYERS = {
        "VK_LAYER_KHRONOS_validation"
};

static constexpr const std::array<std::string_view, 1> REQUIRED_INSTANCE_EXTENSIONS = {
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
};

static constexpr const std::array<std::string_view, 1> REQUIRED_DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

constexpr std::vector<const char *> getRequiredLayersCStr() {
    std::vector<const char *> layers;
    std::transform(REQUIRED_LAYERS.begin(), REQUIRED_LAYERS.end(), std::back_inserter(layers),
                   [](const std::string_view &item) -> const char * { return item.data(); });

    return layers;
}

constexpr std::vector<const char *> getRequiredDeviceExtensionsCStr() {
    std::vector<const char *> extensions;
    std::transform(REQUIRED_DEVICE_EXTENSIONS.begin(), REQUIRED_DEVICE_EXTENSIONS.end(), std::back_inserter(extensions),
                   [](const std::string_view &item) -> const char * { return item.data(); });

    return extensions;
}

#endif // RENDERING_EXTENSIONS_HPP
