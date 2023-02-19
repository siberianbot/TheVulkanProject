#include "RenderingObjectsFactory.hpp"

RenderingObjectsFactory::RenderingObjectsFactory(RenderingDevice *renderingDevice)
        : _renderingDevice(renderingDevice) {
    //
}

BufferObject *RenderingObjectsFactory::createBufferObject(VkDeviceSize size, VkBufferUsageFlags usage,
                                                          VkMemoryPropertyFlags memoryProperty) {
    VkBuffer buffer = this->_renderingDevice->createBuffer(size, usage);
    VkMemoryRequirements requirements = this->_renderingDevice->getBufferMemoryRequirements(buffer);
    VkDeviceMemory memory = this->_renderingDevice->allocateMemory(requirements, memoryProperty);

    this->_renderingDevice->bindBufferMemory(buffer, memory);

    return new BufferObject(this->_renderingDevice, size, buffer, memory);
}
