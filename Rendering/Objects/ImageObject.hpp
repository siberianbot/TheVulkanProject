#ifndef RENDERING_OBJECTS_IMAGEOBJECT_HPP
#define RENDERING_OBJECTS_IMAGEOBJECT_HPP

#include <vulkan/vulkan.hpp>

class RenderingDevice;

class ImageObject {
private:
    RenderingDevice *_renderingDevice;
    VkImage _image;
    VkFormat _format;
    VkDeviceMemory _memory;

public:
    ImageObject(RenderingDevice *renderingDevice, VkImage image, VkFormat format, VkDeviceMemory memory);
    ~ImageObject();

    [[nodiscard]] VkImage getHandle() const { return this->_image; }
    [[nodiscard]] VkFormat getFormat() const { return this->_format; }
};

#endif // RENDERING_OBJECTS_IMAGEOBJECT_HPP
