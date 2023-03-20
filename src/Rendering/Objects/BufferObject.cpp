#include "BufferObject.hpp"

#include "src/Rendering/Common.hpp"
#include "src/Rendering/RenderingDevice.hpp"
#include "src/Rendering/VulkanObjectsAllocator.hpp"

BufferObject::BufferObject(const std::shared_ptr<RenderingDevice> &renderingDevice,
                           const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator,
                           VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize size)
        : _renderingDevice(renderingDevice),
          _vulkanObjectsAllocator(vulkanObjectsAllocator),
          _buffer(buffer),
          _memory(memory),
          _size(size) {
    //
}

void BufferObject::destroy() {
    this->_vulkanObjectsAllocator->freeMemory(this->_memory);
    this->_vulkanObjectsAllocator->destroyBuffer(this->_buffer);
}

void *BufferObject::map() {
    void *ptr = nullptr;
    vkEnsure(vkMapMemory(this->_renderingDevice->getHandle(), this->_memory, 0, this->_size, 0, &ptr));

    return ptr;
}

void BufferObject::unmap() {
    vkUnmapMemory(this->_renderingDevice->getHandle(), this->_memory);
}
