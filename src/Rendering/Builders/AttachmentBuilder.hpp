#ifndef RENDERING_BUILDERS_ATTACHMENTBUILDER_HPP
#define RENDERING_BUILDERS_ATTACHMENTBUILDER_HPP

#include <optional>

#include <vulkan/vulkan.hpp>

class PhysicalDevice;

class AttachmentBuilder {
private:
    PhysicalDevice *_physicalDevice;

    std::optional<VkFormat> _format;
    std::optional<VkSampleCountFlagBits> _samples;
    std::optional<VkAttachmentLoadOp> _loadOp;
    std::optional<VkImageLayout> _initialLayout;
    std::optional<VkImageLayout> _finalLayout;

public:
    explicit AttachmentBuilder(PhysicalDevice *physicalDevice);

    AttachmentBuilder &clear();
    AttachmentBuilder &load();
    AttachmentBuilder &withFormat(VkFormat format);
    AttachmentBuilder &withSamples(VkSampleCountFlagBits samples);
    AttachmentBuilder &withInitialLayout(VkImageLayout layout);
    AttachmentBuilder &withFinalLayout(VkImageLayout layout);

    VkAttachmentDescription build();
};

#endif // RENDERING_BUILDERS_ATTACHMENTBUILDER_HPP
