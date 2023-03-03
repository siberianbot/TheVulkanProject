#ifndef RENDERING_BUILDERS_FRAMEBUFFERSBUILDER_HPP
#define RENDERING_BUILDERS_FRAMEBUFFERSBUILDER_HPP

#include <optional>
#include <vector>

#include <vulkan/vulkan.hpp>

class RenderingDevice;

class FramebufferBuilder {
private:
    RenderingDevice *_renderingDevice;
    VkRenderPass _renderpass;

    std::vector<VkImageView> _attachments;
    std::optional<VkExtent2D> _extent;

public:
    FramebufferBuilder(RenderingDevice *renderingDevice, VkRenderPass renderpass);

    FramebufferBuilder &addAttachment(VkImageView imageView);
    FramebufferBuilder &replaceAttachment(uint32_t idx, VkImageView imageView);
    FramebufferBuilder &withExtent(VkExtent2D extent);

    VkFramebuffer build();
};

#endif // RENDERING_BUILDERS_FRAMEBUFFERSBUILDER_HPP
