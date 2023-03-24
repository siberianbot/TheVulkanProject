#ifndef RENDERING_RENDERPASSES_RENDERPASSBASE_HPP
#define RENDERING_RENDERPASSES_RENDERPASSBASE_HPP

#include <map>
#include <memory>
#include <vector>

#include <vulkan/vulkan.hpp>

class RenderingDevice;
class ImageViewObject;

class RenderpassBase {
private:
    std::vector<VkClearValue> _clearValues;
    std::map<std::shared_ptr<ImageViewObject>, VkFramebuffer> _framebuffers;
    VkFramebuffer _targetFramebuffer;

protected:
    std::shared_ptr<RenderingDevice> _renderingDevice;

    VkRenderPass _renderpass = VK_NULL_HANDLE;
    VkRect2D _targetRenderArea;

    RenderpassBase(const std::shared_ptr<RenderingDevice> &renderingDevice,
                   const std::vector<VkClearValue> &clearValues);

    virtual VkFramebuffer createFramebuffer(const std::shared_ptr<ImageViewObject> &imageView, VkExtent2D extent) = 0;

public:
    virtual ~RenderpassBase() = default;

    [[nodiscard]] VkRenderPass getHandle() { return this->_renderpass; }

    virtual void beginRenderpass(VkCommandBuffer commandBuffer);
    void endRenderpass(VkCommandBuffer commandBuffer);

    virtual void initRenderpass() = 0;
    virtual void destroyRenderpass();

    void destroyFramebuffers();

    void setTargetImageView(const std::shared_ptr<ImageViewObject> &targetImageView, VkRect2D renderArea);
};

#endif // RENDERING_RENDERPASSES_RENDERPASSBASE_HPP
