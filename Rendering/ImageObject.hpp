#ifndef RENDERING_IMAGEOBJECT_HPP
#define RENDERING_IMAGEOBJECT_HPP

#include <vulkan/vulkan.hpp>

class RenderingDevice;

class ImageObject {
private:
    RenderingDevice *_renderingDevice;
    VkImage _image;
    VkDeviceMemory _memory;
    VkImageView _imageView;

public:
    ImageObject(RenderingDevice *renderingDevice, VkImage image, VkDeviceMemory memory, VkImageView imageView);
    ~ImageObject();

    [[deprecated]] [[nodiscard]] VkImage getImageHandle() const { return this->_image; }
    [[deprecated]] [[nodiscard]] VkImageView getImageViewHandle() const { return this->_imageView; }
};

#endif // RENDERING_IMAGEOBJECT_HPP
