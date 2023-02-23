#include "BufferObject.hpp"

#include "src/Rendering/RenderingDevice.hpp"

BufferObject::BufferObject(RenderingDevice *renderingDevice, VkDeviceSize size, VkBuffer buffer, VkDeviceMemory memory)
        : _renderingDevice(renderingDevice),
          _size(size),
          _buffer(buffer),
          _memory(memory) {
    //
}

BufferObject::~BufferObject() {
    this->unmap();
    this->_renderingDevice->freeMemory(this->_memory);
    this->_renderingDevice->destroyBuffer(this->_buffer);
}

void *BufferObject::map() {
    if (this->_mapPtr == nullptr) {
        this->_mapPtr = this->_renderingDevice->mapMemory(this->_memory, this->_size);
    }

    return this->_mapPtr;
}

void BufferObject::unmap() {
    if (this->_mapPtr == nullptr) {
        return;
    }

    this->_renderingDevice->unmapMemory(this->_memory);
    this->_mapPtr = nullptr;
}
