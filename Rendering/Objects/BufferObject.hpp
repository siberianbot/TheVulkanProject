#ifndef RENDERING_BUFFEROBJECT_HPP
#define RENDERING_BUFFEROBJECT_HPP

#include <vulkan/vulkan.hpp>

class RenderingDevice;

class BufferObject {
private:
    RenderingDevice *_renderingDevice;
    VkDeviceSize _size;
    VkBuffer _buffer;
    VkDeviceMemory _memory;

    void *_mapPtr = nullptr;

public:
    BufferObject(RenderingDevice *renderingDevice, VkDeviceSize size, VkBuffer buffer, VkDeviceMemory memory);
    ~BufferObject();

    [[deprecated]] [[nodiscard]] VkBuffer getHandle() const { return this->_buffer; }

    [[nodiscard]] VkDeviceSize getSize() const { return this->_size; }

    void *map(); // TODO: ???
    void unmap();
};

#endif // RENDERING_BUFFEROBJECT_HPP
