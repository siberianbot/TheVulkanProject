#include "RenderingObjectsFactory.hpp"

#include "RenderingDevice.hpp"
#include "src/Rendering/Objects/BufferObject.hpp"
#include "src/Rendering/Objects/DescriptorSetObject.hpp"
#include "src/Rendering/Objects/FenceObject.hpp"
#include "src/Rendering/Objects/ImageObject.hpp"
#include "src/Rendering/Objects/ImageViewObject.hpp"
#include "src/Rendering/Objects/SemaphoreObject.hpp"

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

ImageObject *RenderingObjectsFactory::createImageObject(uint32_t width, uint32_t height, uint32_t layers,
                                                        VkImageCreateFlags flags, VkFormat format,
                                                        VkImageUsageFlags usage, VkSampleCountFlagBits samples,
                                                        VkMemoryPropertyFlags memoryProperty) {
    VkImage image = this->_renderingDevice->createImage(width, height, layers, flags, format, usage, samples);
    VkMemoryRequirements requirements = this->_renderingDevice->getImageMemoryRequirements(image);
    VkDeviceMemory memory = this->_renderingDevice->allocateMemory(requirements, memoryProperty);

    this->_renderingDevice->bindImageMemory(image, memory);

    return new ImageObject(this->_renderingDevice, image, layers, flags, format, memory);
}

FenceObject *RenderingObjectsFactory::createFenceObject(bool signaled) {
    VkFence fence = this->_renderingDevice->createFence(signaled);

    return new FenceObject(this->_renderingDevice, fence);
}

SemaphoreObject *RenderingObjectsFactory::createSemaphoreObject() {
    VkSemaphore semaphore = this->_renderingDevice->createSemaphore();

    return new SemaphoreObject(this->_renderingDevice, semaphore);
}

DescriptorSetObject *RenderingObjectsFactory::createDescriptorSetObject(VkDescriptorPool descriptorPool,
                                                                        VkDescriptorSetLayout descriptorSetLayout,
                                                                        uint32_t descriptorCount) {
    std::vector<VkDescriptorSet> descriptorSets = this->_renderingDevice->allocateDescriptorSets(descriptorCount,
                                                                                                 descriptorPool,
                                                                                                 descriptorSetLayout);

    return new DescriptorSetObject(this->_renderingDevice, descriptorPool, descriptorSets);
}

ImageViewObject *RenderingObjectsFactory::createImageViewObject(VkImage image, VkFormat format,
                                                                VkImageAspectFlags aspectMask) {
    VkImageView imageView = this->_renderingDevice->createImageView(image, 1, VK_IMAGE_VIEW_TYPE_2D, format,
                                                                    aspectMask);

    return new ImageViewObject(this->_renderingDevice, nullptr, imageView);
}

ImageViewObject *RenderingObjectsFactory::createImageViewObject(ImageObject *image, VkImageViewType imageViewType,
                                                                VkImageAspectFlags aspectMask) {
    VkImageView imageView = this->_renderingDevice->createImageView(image->getHandle(), image->getLayersCount(),
                                                                    imageViewType, image->getFormat(), aspectMask);

    return new ImageViewObject(this->_renderingDevice, image, imageView);
}
