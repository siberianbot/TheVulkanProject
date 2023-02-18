// TODO: RENAME THIS FILE

#ifndef ANOTHERVULKANTYPES_HPP
#define ANOTHERVULKANTYPES_HPP

#include <vulkan/vulkan.hpp>

struct RenderingDevice {
    VkDevice device;
    VkFormat colorFormat;
    VkFormat depthFormat;
    VkSampleCountFlagBits samples;
    VkQueue graphicsQueue;
    uint32_t graphicsQueueFamilyIdx;
};

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
