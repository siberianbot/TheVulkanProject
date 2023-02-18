#ifndef RENDERPASSBASE_HPP
#define RENDERPASSBASE_HPP

#include <vector>

#include "AnotherVulkanTypes.hpp"
#include "Rendering/RenderingDevice.hpp"

class RenderpassBase {
protected:
    RenderingDevice *_renderingDevice;

    VkRenderPass _renderpass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> _framebuffers;

public:
    explicit RenderpassBase(RenderingDevice *renderingDevice);
    virtual ~RenderpassBase() = default;

    virtual void recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                                uint32_t frameIdx, uint32_t imageIdx) = 0;

    virtual void initRenderpass() = 0;
    virtual void destroyRenderpass();

    virtual void createFramebuffers(const Swapchain &swapchain, const RenderTargets &renderTargets);
    void destroyFramebuffers();
};

#endif // RENDERPASSBASE_HPP
