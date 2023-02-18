#ifndef RENDERING_VULKANFRAMEBUFFERSBUILDER_HPP
#define RENDERING_VULKANFRAMEBUFFERSBUILDER_HPP

#include "AnotherVulkanTypes.hpp"

#include "Rendering/RenderingDevice.hpp"

// TODO naming - remove Vulkan prefix
class VulkanFramebuffersBuilder {
private:
    RenderingDevice *_renderingDevice;
    Swapchain _swapchain;
    RenderTargets _renderTargets;
    VkRenderPass _renderpass;

    bool _withResolveTargets = false;

public:
    VulkanFramebuffersBuilder(RenderingDevice *renderingDevice,
                              const Swapchain &swapchain,
                              const RenderTargets &renderTargets,
                              VkRenderPass renderpass);

    VulkanFramebuffersBuilder &withResolveTargets();

    std::vector<VkFramebuffer> build();
};

#endif // RENDERING_VULKANFRAMEBUFFERSBUILDER_HPP
