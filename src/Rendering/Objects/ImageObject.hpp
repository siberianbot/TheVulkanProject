#ifndef RENDERING_OBJECTS_IMAGEOBJECT_HPP
#define RENDERING_OBJECTS_IMAGEOBJECT_HPP

#include <memory>

#include <vulkan/vulkan.hpp>

class VulkanObjectsAllocator;

class ImageObject {
private:
    std::shared_ptr<VulkanObjectsAllocator> _vulkanObjectsAllocator;

    VkImage _image;
    VkDeviceMemory _memory;
    uint32_t _layers;
    VkFormat _format;

public:
    ImageObject(const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator,
                VkImage image, VkDeviceMemory memory, uint32_t layers, VkFormat format);

    [[nodiscard]] VkImage getHandle() const { return this->_image; }
    [[nodiscard]] uint32_t getLayersCount() const { return this->_layers; }
    [[nodiscard]] VkFormat getFormat() const { return this->_format; }

    void destroy();
};

#endif // RENDERING_OBJECTS_IMAGEOBJECT_HPP
