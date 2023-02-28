#ifndef RENDERING_BUILDERS_FRAMEBUFFERSBUILDER_HPP
#define RENDERING_BUILDERS_FRAMEBUFFERSBUILDER_HPP

#include <vector>

#include <vulkan/vulkan.hpp>

class RenderingDevice;
class Swapchain;

class FramebuffersBuilder {
private:
    RenderingDevice *_renderingDevice;
    Swapchain *_swapchain;
    VkRenderPass _renderpass;

    std::vector<VkImageView> _attachments;

public:
    FramebuffersBuilder(RenderingDevice *renderingDevice,
                        Swapchain *swapchain,
                        VkRenderPass renderpass);

    FramebuffersBuilder &addAttachment(VkImageView imageView);

    std::vector<VkFramebuffer> build();
};

#endif // RENDERING_BUILDERS_FRAMEBUFFERSBUILDER_HPP
