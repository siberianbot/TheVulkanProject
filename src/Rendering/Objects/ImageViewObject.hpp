#ifndef RENDERING_OBJECTS_IMAGEVIEWOBJECT_HPP
#define RENDERING_OBJECTS_IMAGEVIEWOBJECT_HPP

#include <vulkan/vulkan.hpp>

class RenderingDevice;
class ImageObject;

class ImageViewObject {
private:
    RenderingDevice *_renderingDevice;
    ImageObject *_image;
    VkImageView _imageView;

public:
    ImageViewObject(RenderingDevice *renderingDevice, ImageObject *image, VkImageView imageView);
    ~ImageViewObject();

    [[nodiscard]] VkImageView getHandle() const { return this->_imageView; }
    [[nodiscard]] ImageObject *getImage() const { return this->_image; }
};

#endif // RENDERING_OBJECTS_IMAGEVIEWOBJECT_HPP
