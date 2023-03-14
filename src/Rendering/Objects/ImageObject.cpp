#include "ImageObject.hpp"

#include "src/Rendering/RenderingDevice.hpp"

ImageObject::ImageObject(RenderingDevice *renderingDevice, VkImage image, uint32_t layers, VkImageCreateFlags flags,
                         VkFormat format, VkDeviceMemory memory)
        : _renderingDevice(renderingDevice),
          _image(image),
          _layers(layers),
          _flags(flags),
          _format(format),
          _memory(memory) {
    //
}

ImageObject::~ImageObject() {
    this->_renderingDevice->freeMemory(this->_memory);
    this->_renderingDevice->destroyImage(this->_image);
}

ImageObject *ImageObject::create(RenderingDevice *renderingDevice, uint32_t width, uint32_t height, uint32_t layers,
                                 VkImageCreateFlags flags, VkFormat format, VkImageUsageFlags usage,
                                 VkSampleCountFlagBits samples, VkMemoryPropertyFlags memoryProperty) {
    VkImage image = renderingDevice->createImage(width, height, layers, flags, format, usage, samples);
    VkMemoryRequirements requirements = renderingDevice->getImageMemoryRequirements(image);
    VkDeviceMemory memory = renderingDevice->allocateMemory(requirements, memoryProperty);

    renderingDevice->bindImageMemory(image, memory);

    return new ImageObject(renderingDevice, image, layers, flags, format, memory);
}
