#include "ImageViewObject.hpp"

#include "src/Rendering/VulkanObjectsAllocator.hpp"

ImageViewObject::ImageViewObject(const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator, VkImage image,
                                 VkImageView imageView)
        : _vulkanObjectsAllocator(vulkanObjectsAllocator),
          _image(image),
          _imageView(imageView) {
    //
}

void ImageViewObject::destroy() {
    this->_vulkanObjectsAllocator->destroyImageView(this->_imageView);
}
