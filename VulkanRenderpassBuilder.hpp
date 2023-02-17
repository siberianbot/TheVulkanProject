#ifndef VULKANRENDERPASSBUILDER_HPP
#define VULKANRENDERPASSBUILDER_HPP

#include <optional>
#include <vector>

#include "AnotherVulkanTypes.hpp"

class VulkanRenderpassBuilder {
private:
    struct Attachment {
        VkImageLayout layout;
        VkImageLayout initialLayout;
        VkImageLayout finalLayout;
    };

    RenderingDevice _renderingDevice;

    std::optional<VkAttachmentLoadOp> _loadOp;
    std::optional<Attachment> _colorAttachment;
    std::optional<Attachment> _depthAttachment;
    std::optional<Attachment> _resolveAttachment;

public:
    explicit VulkanRenderpassBuilder(const RenderingDevice &renderingDevice);

    VulkanRenderpassBuilder &clear();
    VulkanRenderpassBuilder &load();
    VulkanRenderpassBuilder &addResolveAttachment();

    VkRenderPass build();
};

#endif // VULKANRENDERPASSBUILDER_HPP
