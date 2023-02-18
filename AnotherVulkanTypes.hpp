// TODO: RENAME THIS FILE

#ifndef ANOTHERVULKANTYPES_HPP
#define ANOTHERVULKANTYPES_HPP

#include <vulkan/vulkan.hpp>

struct Swapchain {
    uint32_t width;
    uint32_t height;
    uint32_t swapchainImagesCount;
};

struct RenderTargets {
    std::vector<VkImageView> colorGroup;
    std::vector<VkImageView> depthGroup;
    std::vector<VkImageView> resolveGroup;
};

#endif // ANOTHERVULKANTYPES_HPP
