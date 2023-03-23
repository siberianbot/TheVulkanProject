#ifndef RENDERING_RENDERPASSES_RENDERPASSBASE_HPP
#define RENDERING_RENDERPASSES_RENDERPASSBASE_HPP

#include <memory>
#include <vector>

#include <vulkan/vulkan.hpp>

class RenderingDevice;
class ImageViewObject;

class RenderpassBase {
protected:
    VkRenderPass _renderpass = VK_NULL_HANDLE;
//    std::shared_ptr<RenderingDevice> _renderingDevice;
//
//    std::vector<VkFramebuffer> _framebuffers;
//
//    RenderpassBase(const std::shared_ptr<RenderingDevice> &renderingDevice);

public:
    virtual ~RenderpassBase() = default;

    [[nodiscard]] VkRenderPass getHandle() { return this->_renderpass; }

    // TODO:
//    virtual void record(VkCommandBuffer commandBuffer, VkRect2D renderArea, uint32_t imagf) = 0;
//
//    virtual void initRenderpass() = 0;
//    virtual void destroyRenderpass();
//
//    virtual void destroyFramebuffers();
//
//    virtual VkFramebuffer createFramebuffer(const std::shared_ptr<ImageViewObject> &targetImageView) = 0;
//
//    void setTargetImageViews(const std::vector<std::shared_ptr<ImageViewObject>> &targetImageViews);
};

#endif // RENDERING_RENDERPASSES_RENDERPASSBASE_HPP
