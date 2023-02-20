#include "ImageObject.hpp"

#include "Rendering/RenderingDevice.hpp"

ImageObject::ImageObject(RenderingDevice *renderingDevice, VkImage image, VkImageView imageView)
        : _renderingDevice(renderingDevice),
          _image(image),
          _memory(VK_NULL_HANDLE),
          _imageView(imageView),
          _fromSwapchain(true) {
    //
}

ImageObject::ImageObject(RenderingDevice *renderingDevice, VkImage image, VkDeviceMemory memory,
                         VkImageView imageView)
        : _renderingDevice(renderingDevice),
          _image(image),
          _memory(memory),
          _imageView(imageView),
          _fromSwapchain(false) {
    //
}

ImageObject::~ImageObject() {
    this->_renderingDevice->destroyImageView(this->_imageView);

    if (!this->_fromSwapchain) {
        this->_renderingDevice->freeMemory(this->_memory);
        this->_renderingDevice->destroyImage(this->_image);
    }
}
