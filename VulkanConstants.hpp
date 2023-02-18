#ifndef VULKANCONSTANTS_HPP
#define VULKANCONSTANTS_HPP

#include <cstdint>

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

#endif // VULKANCONSTANTS_HPP
