#ifndef RENDERING_RENDERPASSES_RENDERPASSBASE_HPP
#define RENDERING_RENDERPASSES_RENDERPASSBASE_HPP

#include <vector>

#include <vulkan/vulkan.hpp>

class RenderingDevice;
class Swapchain;

class RenderpassBase {
protected:
    RenderingDevice *_renderingDevice;
    Swapchain *_swapchain;

    VkRenderPass _renderpass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> _framebuffers;

public:
    explicit RenderpassBase(RenderingDevice *renderingDevice, Swapchain *swapchain);
    virtual ~RenderpassBase() = default;

    virtual void recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                                uint32_t frameIdx, uint32_t imageIdx) = 0;

    virtual void initRenderpass() = 0;
    virtual void destroyRenderpass();

    virtual void createFramebuffers();
    void destroyFramebuffers();
};

#endif // RENDERING_RENDERPASSES_RENDERPASSBASE_HPP
