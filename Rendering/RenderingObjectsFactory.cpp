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

ImageObject *RenderingObjectsFactory::createImageObject(uint32_t width, uint32_t height, VkFormat format,
                                                        VkImageUsageFlags usage, VkSampleCountFlagBits samples,
                                                        VkMemoryPropertyFlags memoryProperty,
                                                        VkImageAspectFlags aspectMask) {
    VkImage image = this->_renderingDevice->createImage(width, height, format, usage, samples);
    VkMemoryRequirements requirements = this->_renderingDevice->getImageMemoryRequirements(image);
    VkDeviceMemory memory = this->_renderingDevice->allocateMemory(requirements, memoryProperty);

    this->_renderingDevice->bindImageMemory(image, memory);

    VkImageView imageView = this->_renderingDevice->createImageView(image, format, aspectMask);

    return new ImageObject(this->_renderingDevice, image, memory, imageView);
}
