#ifndef RENDERING_OBJECTS_IMAGEOBJECT_HPP
#define RENDERING_OBJECTS_IMAGEOBJECT_HPP

#include <vulkan/vulkan.hpp>

class RenderingDevice;

class ImageObject {
private:
    RenderingDevice *_renderingDevice;
    VkImage _image;
    uint32_t _layers;
    VkImageCreateFlags _flags;
    VkFormat _format;
    VkDeviceMemory _memory;

public:
    ImageObject(RenderingDevice *renderingDevice, VkImage image, uint32_t layers, VkImageCreateFlags flags,
                VkFormat format, VkDeviceMemory memory);
    ~ImageObject();

    [[nodiscard]] VkImage getHandle() const { return this->_image; }
    [[nodiscard]] uint32_t getLayersCount() const { return this->_layers; }
    [[nodiscard]] VkImageCreateFlags getFlags() const { return this->_flags; }
    [[nodiscard]] VkFormat getFormat() const { return this->_format; }
};

#endif // RENDERING_OBJECTS_IMAGEOBJECT_HPP
