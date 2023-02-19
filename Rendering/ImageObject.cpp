#include "ImageObject.hpp"

#include "Rendering/RenderingDevice.hpp"

ImageObject::ImageObject(RenderingDevice *renderingDevice, VkImage image, VkDeviceMemory memory,
                         VkImageView imageView)
        : _renderingDevice(renderingDevice),
          _image(image),
          _memory(memory),
          _imageView(imageView) {
    //
}

ImageObject::~ImageObject() {
    this->_renderingDevice->destroyImageView(this->_imageView);
    this->_renderingDevice->freeMemory(this->_memory);
    this->_renderingDevice->destroyImage(this->_image);
}
