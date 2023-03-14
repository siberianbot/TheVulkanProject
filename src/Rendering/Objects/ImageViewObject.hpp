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

    ImageViewObject(RenderingDevice *renderingDevice, ImageObject *image, VkImageView imageView);

public:
    ~ImageViewObject();

    [[nodiscard]] ImageObject *getImage() const { return this->_image; }
    [[nodiscard]] VkImageView getHandle() const { return this->_imageView; }

    [[nodiscard]] static ImageViewObject *create(RenderingDevice *renderingDevice, VkImage image, VkFormat format,
                                                 VkImageAspectFlags aspectMask);
    [[nodiscard]] static ImageViewObject *create(RenderingDevice *renderingDevice, ImageObject *image,
                                                 VkImageViewType imageViewType, VkImageAspectFlags aspectMask);
};

#endif // RENDERING_OBJECTS_IMAGEVIEWOBJECT_HPP
