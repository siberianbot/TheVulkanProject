#include "ImageViewObject.hpp"

#include "src/Rendering/RenderingDevice.hpp"

ImageViewObject::ImageViewObject(RenderingDevice *renderingDevice, ImageObject *image, VkImageView imageView)
        : _renderingDevice(renderingDevice),
          _image(image),
          _imageView(imageView) {
    //
}

ImageViewObject::~ImageViewObject() {
    this->_renderingDevice->destroyImageView(this->_imageView);
}
