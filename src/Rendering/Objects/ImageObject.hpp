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

    ImageObject(RenderingDevice *renderingDevice, VkImage image, uint32_t layers, VkImageCreateFlags flags,
                VkFormat format, VkDeviceMemory memory);

public:
    ~ImageObject();

    [[nodiscard]] VkImage getHandle() const { return this->_image; }
    [[nodiscard]] uint32_t getLayersCount() const { return this->_layers; }
    [[nodiscard]] VkImageCreateFlags getFlags() const { return this->_flags; }
    [[nodiscard]] VkFormat getFormat() const { return this->_format; }

    [[nodiscard]] static ImageObject *create(RenderingDevice *renderingDevice, uint32_t width, uint32_t height,
                                             uint32_t layers, VkImageCreateFlags flags, VkFormat format,
                                             VkImageUsageFlags usage, VkSampleCountFlagBits samples,
                                             VkMemoryPropertyFlags memoryProperty);
};

#endif // RENDERING_OBJECTS_IMAGEOBJECT_HPP
