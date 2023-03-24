#ifndef RENDERING_RENDERPASSES_IMGUIRENDERPASS_HPP
#define RENDERING_RENDERPASSES_IMGUIRENDERPASS_HPP

#include "RenderpassBase.hpp"

class DebugUIRenderpass : public RenderpassBase {
private:
    VkFramebuffer createFramebuffer(const std::shared_ptr<ImageViewObject> &imageView, VkExtent2D extent) override;

public:
    DebugUIRenderpass(const std::shared_ptr<RenderingDevice> &renderingDevice);
    ~DebugUIRenderpass() override = default;

    void record(VkCommandBuffer commandBuffer);

    void initRenderpass() override;
};

#endif // RENDERING_RENDERPASSES_IMGUIRENDERPASS_HPP
