#include "ImageObject.hpp"

#include "src/Rendering/VulkanObjectsAllocator.hpp"

ImageObject::ImageObject(const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator, VkImage image,
                         VkDeviceMemory memory, uint32_t layers, VkFormat format)
        : _vulkanObjectsAllocator(vulkanObjectsAllocator),
          _image(image),
          _memory(memory),
          _layers(layers),
          _format(format) {
    //
}

void ImageObject::destroy() {
    this->_vulkanObjectsAllocator->freeMemory(this->_memory);
    this->_vulkanObjectsAllocator->destroyImage(this->_image);
}
