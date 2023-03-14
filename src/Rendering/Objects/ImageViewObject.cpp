#include "ImageViewObject.hpp"

#include "src/Rendering/RenderingDevice.hpp"
#include "src/Rendering/Objects/ImageObject.hpp"

ImageViewObject::ImageViewObject(RenderingDevice *renderingDevice, ImageObject *image, VkImageView imageView)
        : _renderingDevice(renderingDevice),
          _image(image),
          _imageView(imageView) {
    //
}

ImageViewObject::~ImageViewObject() {
    this->_renderingDevice->destroyImageView(this->_imageView);
}

ImageViewObject *ImageViewObject::create(RenderingDevice *renderingDevice, VkImage image, VkFormat format,
                                         VkImageAspectFlags aspectMask) {
    VkImageView imageView = renderingDevice->createImageView(image, 1, VK_IMAGE_VIEW_TYPE_2D, format,
                                                             aspectMask);

    return new ImageViewObject(renderingDevice, nullptr, imageView);
}

ImageViewObject *ImageViewObject::create(RenderingDevice *renderingDevice, ImageObject *image,
                                         VkImageViewType imageViewType, VkImageAspectFlags aspectMask) {
    VkImageView imageView = renderingDevice->createImageView(image->getHandle(), image->getLayersCount(),
                                                             imageViewType, image->getFormat(), aspectMask);

    return new ImageViewObject(renderingDevice, image, imageView);
}
