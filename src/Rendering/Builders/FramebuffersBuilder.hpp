#ifndef RENDERING_BUILDERS_FRAMEBUFFERSBUILDER_HPP
#define RENDERING_BUILDERS_FRAMEBUFFERSBUILDER_HPP

#include <vulkan/vulkan.hpp>

class RenderingDevice;
class Swapchain;

class FramebuffersBuilder {
private:
    RenderingDevice *_renderingDevice;
    Swapchain *_swapchain;
    VkRenderPass _renderpass;

    bool _withDepthTargets = false;
    bool _withResolveTargets = false;

public:
    FramebuffersBuilder(RenderingDevice *renderingDevice,
                        Swapchain *swapchain,
                        VkRenderPass renderpass);

    FramebuffersBuilder &withDepthTargets();
    FramebuffersBuilder &withResolveTargets();

    std::vector<VkFramebuffer> build();
};

#endif // RENDERING_BUILDERS_FRAMEBUFFERSBUILDER_HPP
