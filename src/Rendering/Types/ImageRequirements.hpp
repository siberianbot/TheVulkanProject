#ifndef RENDERING_TYPES_IMAGEREQUIREMENTS_HPP
#define RENDERING_TYPES_IMAGEREQUIREMENTS_HPP

#include <optional>

#include <vulkan/vulkan.hpp>

struct ImageRequirements {
    vk::ImageUsageFlags usage;
    vk::MemoryPropertyFlags memoryProperties;

    vk::Extent3D extent;
    vk::Format format;

    std::optional<uint32_t> layerCount;
    std::optional<vk::SampleCountFlagBits> samples;
    std::optional<vk::ImageCreateFlags> imageFlags;

    std::optional<vk::ImageViewType> type;
    std::optional<vk::ImageAspectFlags> aspectMask;
};

#endif // RENDERING_TYPES_IMAGEREQUIREMENTS_HPP
