#ifndef RENDERING_BUILDERS_IMAGEOBJECTBUILDER_HPP
#define RENDERING_BUILDERS_IMAGEOBJECTBUILDER_HPP

#include <memory>
#include <optional>

#include <vulkan/vulkan.hpp>

class RenderingDevice;
class VulkanObjectsAllocator;
class ImageObject;

class ImageObjectBuilder {
private:
    std::shared_ptr<RenderingDevice> _renderingDevice;
    std::shared_ptr<VulkanObjectsAllocator> _vulkanObjectsAllocator;

    std::optional<VkImageCreateFlags> _flags;
    VkExtent3D _extent;
    VkFormat _format = VK_FORMAT_UNDEFINED;
    std::optional<uint32_t> _count;
    std::optional<VkSampleCountFlagBits> _samples;
    std::optional<VkImageUsageFlags> _usage;

public:
    ImageObjectBuilder(const std::shared_ptr<RenderingDevice> &renderingDevice,
                       const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator);

    ImageObjectBuilder &withExtent(uint32_t width, uint32_t height);
    ImageObjectBuilder &withFormat(VkFormat format);
    ImageObjectBuilder &withSamples(VkSampleCountFlagBits samples);
    ImageObjectBuilder &withUsage(VkImageUsageFlags usage);

    ImageObjectBuilder &cubeCompatible();
    ImageObjectBuilder &arrayCompatible(uint32_t count);

    std::shared_ptr<ImageObject> build();
};

#endif // RENDERING_BUILDERS_IMAGEOBJECTBUILDER_HPP
