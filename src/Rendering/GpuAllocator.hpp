// TODO:
//  [ ] stack-based allocation

#ifndef RENDERING_GPUALLOCATOR_HPP
#define RENDERING_GPUALLOCATOR_HPP

#include <memory>
#include <optional>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "src/Rendering/Types/BufferView.hpp"
#include "src/Rendering/Types/ImageView.hpp"

class Log;
class LogicalDeviceProxy;
class PhysicalDeviceProxy;

struct BufferRequirements {
    vk::DeviceSize size;
    vk::BufferUsageFlags usage;
    vk::MemoryPropertyFlags memoryProperties;
};

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

class GpuAllocator {
private:
    struct BufferAllocation {
        BufferRequirements requirements;
        vk::Buffer buffer;
        vk::DeviceMemory memory;
    };

    struct ImageAllocation {
        ImageRequirements requirements;
        vk::Image image;
        vk::ImageView imageView;
        vk::DeviceMemory memory;
    };

    std::shared_ptr<Log> _log;
    std::shared_ptr<PhysicalDeviceProxy> _physicalDevice;
    std::shared_ptr<LogicalDeviceProxy> _logicalDevice;

    std::vector<BufferAllocation> _buffers;
    std::vector<std::shared_ptr<BufferView>> _bufferViews;

    std::vector<ImageAllocation> _images;
    std::vector<std::shared_ptr<ImageView>> _imageViews;

    vk::DeviceMemory allocateMemory(const vk::MemoryRequirements &requirements,
                                    vk::MemoryPropertyFlags properties);

    BufferAllocation createBufferAllocation(const BufferRequirements &requirements);
    void freeBufferAllocation(const BufferAllocation &allocation);

    ImageAllocation createImageAllocation(const ImageRequirements &requirements);
    void freeImageAllocation(const ImageAllocation &allocation);

    std::shared_ptr<BufferView> createBufferView(const BufferAllocation &allocation);
    std::shared_ptr<ImageView> createImageView(const ImageAllocation &allocation);

public:
    GpuAllocator(const std::shared_ptr<Log> &log,
                 const std::shared_ptr<PhysicalDeviceProxy> &physicalDevice,
                 const std::shared_ptr<LogicalDeviceProxy> &logicalDevice);

    [[nodiscard]] std::weak_ptr<BufferView> allocateBuffer(const BufferRequirements &requirements, bool map);
    [[nodiscard]] std::weak_ptr<ImageView> allocateImage(const ImageRequirements &requirements);

    void freeBuffer(const std::weak_ptr<BufferView> &bufferView);
    void freeImage(const std::weak_ptr<ImageView> &imageView);

    void freeAll();
};

#endif // RENDERING_GPUALLOCATOR_HPP
