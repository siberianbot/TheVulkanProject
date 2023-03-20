#ifndef RENDERING_BUILDERS_IMAGEVIEWOBJECTBUILDER_HPP
#define RENDERING_BUILDERS_IMAGEVIEWOBJECTBUILDER_HPP

#include <memory>
#include <optional>

#include <vulkan/vulkan.hpp>

class VulkanObjectsAllocator;
class ImageObject;
class ImageViewObject;

class ImageViewObjectBuilder {
private:
    std::shared_ptr<VulkanObjectsAllocator> _vulkanObjectsAllocator;

    VkImage _image = VK_NULL_HANDLE;
    VkFormat _format = VK_FORMAT_UNDEFINED;
    std::optional<VkImageViewType> _type;
    std::optional<VkImageAspectFlags> _aspectMask;
    std::optional<uint32_t> _baseArrayLayer;
    std::optional<uint32_t> _layerCount;

public:
    ImageViewObjectBuilder(const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator);

    ImageViewObjectBuilder &fromImageObject(const std::shared_ptr<ImageObject> &imageObject);

    ImageViewObjectBuilder &withImage(VkImage image);
    ImageViewObjectBuilder &withFormat(VkFormat format);
    ImageViewObjectBuilder &withType(VkImageViewType type);
    ImageViewObjectBuilder &withAspectFlags(VkImageAspectFlags aspectMask);
    ImageViewObjectBuilder &withLayers(uint32_t baseLayer, uint32_t count);

    std::shared_ptr<ImageViewObject> build();
};

#endif // RENDERING_BUILDERS_IMAGEVIEWOBJECTBUILDER_HPP
