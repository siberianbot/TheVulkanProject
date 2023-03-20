#ifndef RENDERING_BUILDERS_BUFFEROBJECTBUILDER_HPP
#define RENDERING_BUILDERS_BUFFEROBJECTBUILDER_HPP

#include <memory>
#include <optional>

#include <vulkan/vulkan.hpp>

class RenderingDevice;
class VulkanObjectsAllocator;
class BufferObject;

class BufferObjectBuilder {
private:
    std::shared_ptr<RenderingDevice> _renderingDevice;
    std::shared_ptr<VulkanObjectsAllocator> _vulkanObjectsAllocator;

    std::optional<VkDeviceSize> _size;
    std::optional<VkBufferUsageFlags> _usage;
    std::optional<VkMemoryPropertyFlags> _memoryProperties;

public:
    BufferObjectBuilder(const std::shared_ptr<RenderingDevice> &renderingDevice,
                        const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator);

    BufferObjectBuilder &withSize(VkDeviceSize size);
    BufferObjectBuilder &withUsage(VkBufferUsageFlags usage);

    BufferObjectBuilder &hostAvailable();
    BufferObjectBuilder &deviceLocal();

    std::shared_ptr<BufferObject> build();
};

#endif // RENDERING_BUILDERS_BUFFEROBJECTBUILDER_HPP
