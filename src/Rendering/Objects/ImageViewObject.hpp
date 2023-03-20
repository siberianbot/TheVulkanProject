#ifndef RENDERING_OBJECTS_IMAGEVIEWOBJECT_HPP
#define RENDERING_OBJECTS_IMAGEVIEWOBJECT_HPP

#include <memory>

#include <vulkan/vulkan.hpp>

class VulkanObjectsAllocator;

class ImageViewObject {
private:
    std::shared_ptr<VulkanObjectsAllocator> _vulkanObjectsAllocator;

    VkImage _image;
    VkImageView _imageView;

public:
    ImageViewObject(const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator,
                    VkImage image, VkImageView imageView);

    [[nodiscard]] VkImage getImage() const { return this->_image; }
    [[nodiscard]] VkImageView getHandle() const { return this->_imageView; }

    void destroy();
};

#endif // RENDERING_OBJECTS_IMAGEVIEWOBJECT_HPP
