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
    bool _fromSwapchain;

public:
    // TODO: it is not obvious that first ctor creates swapchain image proxy
    ImageObject(RenderingDevice *renderingDevice, VkImage image, VkImageView imageView);
    ImageObject(RenderingDevice *renderingDevice, VkImage image, VkDeviceMemory memory,
                VkImageView imageView);
    ~ImageObject();

    [[nodiscard]] VkImage getImageHandle() const { return this->_image; }
    [[nodiscard]] VkImageView getImageViewHandle() const { return this->_imageView; }
};

#endif // RENDERING_IMAGEOBJECT_HPP
