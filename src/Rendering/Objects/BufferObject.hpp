#ifndef RENDERING_OBJECTS_BUFFEROBJECT_HPP
#define RENDERING_OBJECTS_BUFFEROBJECT_HPP

#include <memory>

#include <vulkan/vulkan.hpp>

class RenderingDevice;
class VulkanObjectsAllocator;

class BufferObject {
private:
    std::shared_ptr<RenderingDevice> _renderingDevice;
    std::shared_ptr<VulkanObjectsAllocator> _vulkanObjectsAllocator;

    VkBuffer _buffer;
    VkDeviceMemory _memory;
    VkDeviceSize _size;

public:
    BufferObject(const std::shared_ptr<RenderingDevice> &renderingDevice,
                 const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator,
                 VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize size);

    [[nodiscard]] VkBuffer getHandle() const { return this->_buffer; }
    [[nodiscard]] VkDeviceSize getSize() const { return this->_size; }

    void destroy();

    [[nodiscard]] void *map();
    void unmap();
};

#endif // RENDERING_OBJECTS_BUFFEROBJECT_HPP
