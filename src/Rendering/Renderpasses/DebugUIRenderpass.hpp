#ifndef RENDERING_RENDERPASSES_IMGUIRENDERPASS_HPP
#define RENDERING_RENDERPASSES_IMGUIRENDERPASS_HPP

#include <map>

#include "RenderpassBase.hpp"

class PhysicalDevice;
class CommandExecutor;
class Swapchain;
class VulkanObjectsAllocator;
class ImageViewObject;

class DebugUIRenderpass : public RenderpassBase {
private:
    std::shared_ptr<RenderingDevice> _renderingDevice;

    std::map<std::shared_ptr<ImageViewObject>, VkFramebuffer> _framebuffers;
    std::shared_ptr<ImageViewObject> _targetImageView = nullptr;

    VkFramebuffer createFramebuffer(const std::shared_ptr<ImageViewObject> &imageView, VkExtent2D extent);

public:
    DebugUIRenderpass(const std::shared_ptr<RenderingDevice> &renderingDevice);
    ~DebugUIRenderpass() override = default;

    void beginRenderpass(VkCommandBuffer commandBuffer, VkRect2D renderArea, uint32_t frameIdx, uint32_t imageIdx);
    void endRenderpass(VkCommandBuffer commandBuffer);

    void record(VkCommandBuffer commandBuffer);

    void initRenderpass();
    void destroyRenderpass();
    void destroyFramebuffers();

    void setTargetImageView(const std::shared_ptr<ImageViewObject> &targetImageView);
};

#endif // RENDERING_RENDERPASSES_IMGUIRENDERPASS_HPP
