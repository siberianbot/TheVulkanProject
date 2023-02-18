#ifndef RENDERING_VULKANRENDERPASSBUILDER_HPP
#define RENDERING_VULKANRENDERPASSBUILDER_HPP

#include <optional>
#include <vector>

#include "Rendering/RenderingDevice.hpp"

// TODO naming - remove Vulkan prefix
class VulkanRenderpassBuilder {
private:
    struct Attachment {
        VkImageLayout layout;
        VkImageLayout initialLayout;
        VkImageLayout finalLayout;
    };

    RenderingDevice *_renderingDevice;

    std::optional<VkAttachmentLoadOp> _loadOp;
    std::optional<Attachment> _colorAttachment;
    std::optional<Attachment> _depthAttachment;
    std::optional<Attachment> _resolveAttachment;

public:
    explicit VulkanRenderpassBuilder(RenderingDevice *renderingDevice);

    VulkanRenderpassBuilder &clear();
    VulkanRenderpassBuilder &load();
    VulkanRenderpassBuilder &addResolveAttachment();

    VkRenderPass build();
};

#endif // RENDERING_VULKANRENDERPASSBUILDER_HPP
