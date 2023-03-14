#ifndef RENDERING_OBJECTS_BUFFEROBJECT_HPP
#define RENDERING_OBJECTS_BUFFEROBJECT_HPP

#include <vulkan/vulkan.hpp>

class RenderingDevice;

class BufferObject {
private:
    RenderingDevice *_renderingDevice;

    VkDeviceSize _size;
    VkBuffer _buffer;
    VkDeviceMemory _memory;

    void *_mapPtr = nullptr;

    BufferObject(RenderingDevice *renderingDevice, VkDeviceSize size, VkBuffer buffer, VkDeviceMemory memory);

public:
    ~BufferObject();

    [[nodiscard]] VkBuffer getHandle() const { return this->_buffer; }
    [[nodiscard]] VkDeviceSize getSize() const { return this->_size; }

    [[nodiscard]] void *map();
    void unmap();

    [[nodiscard]] static BufferObject *create(RenderingDevice *renderingDevice, VkDeviceSize size,
                                              VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperty);
};

#endif // RENDERING_OBJECTS_BUFFEROBJECT_HPP
