#ifndef RENDERING_RENDERPASSBUILDER_HPP
#define RENDERING_RENDERPASSBUILDER_HPP

#include <optional>
#include <vector>

#include <vulkan/vulkan.hpp>

class RenderingDevice;

class RenderpassBuilder {
private:
    struct Attachment {
        VkImageLayout layout;
        VkImageLayout initialLayout;
        VkImageLayout finalLayout;
    };

    RenderingDevice *_renderingDevice;

    bool _noDepthAttachment = false;
    std::optional<VkAttachmentLoadOp> _loadOp;
    std::optional<Attachment> _colorAttachment;
    std::optional<Attachment> _depthAttachment;
    std::optional<Attachment> _resolveAttachment;

public:
    explicit RenderpassBuilder(RenderingDevice *renderingDevice);

    RenderpassBuilder &noDepthAttachment();
    RenderpassBuilder &clear();
    RenderpassBuilder &load();
    RenderpassBuilder &addResolveAttachment();

    VkRenderPass build();
};

#endif // RENDERING_RENDERPASSBUILDER_HPP
