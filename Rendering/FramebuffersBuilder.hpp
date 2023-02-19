#ifndef RENDERING_FRAMEBUFFERSBUILDER_HPP
#define RENDERING_FRAMEBUFFERSBUILDER_HPP

#include <vulkan/vulkan.hpp>

class RenderingDevice;
class Swapchain;

class FramebuffersBuilder {
private:
    RenderingDevice *_renderingDevice;
    Swapchain *_swapchain;
    VkRenderPass _renderpass;

    bool _withResolveTargets = false;

public:
    FramebuffersBuilder(RenderingDevice *renderingDevice,
                        Swapchain *swapchain,
                        VkRenderPass renderpass);

    FramebuffersBuilder &withResolveTargets();

    std::vector<VkFramebuffer> build();
};

#endif // RENDERING_FRAMEBUFFERSBUILDER_HPP
