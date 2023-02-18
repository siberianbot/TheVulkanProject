// TODO: RENAME THIS FILE

#ifndef ANOTHERVULKANTYPES_HPP
#define ANOTHERVULKANTYPES_HPP

#include <vulkan/vulkan.hpp>

struct RenderingDevice {
    VkDevice device;
    VkFormat colorFormat; // TODO part of physical device
    VkFormat depthFormat; // TODO part of physical device
    VkSampleCountFlagBits samples; // TODO part of physical device
    VkQueue graphicsQueue;
    uint32_t graphicsQueueFamilyIdx; // TODO part of physical device
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
