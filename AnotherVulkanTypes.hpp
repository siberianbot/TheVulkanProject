// TODO: RENAME THIS FILE

#ifndef ANOTHERVULKANTYPES_HPP
#define ANOTHERVULKANTYPES_HPP

#include <vulkan/vulkan.hpp>

struct RenderingDevice {
    VkDevice device;
    VkFormat colorFormat;
    VkFormat depthFormat;
    VkSampleCountFlagBits samples;
};

#endif // ANOTHERVULKANTYPES_HPP
