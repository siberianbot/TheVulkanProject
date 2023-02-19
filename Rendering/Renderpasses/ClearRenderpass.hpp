#ifndef RENDERING_RENDERPASSES_CLEARRENDERPASS_HPP
#define RENDERING_RENDERPASSES_CLEARRENDERPASS_HPP

#include "RenderpassBase.hpp"

class ClearRenderpass : public RenderpassBase {
public:
    ClearRenderpass(RenderingDevice *renderingDevice, Swapchain *swapchain);
    ~ClearRenderpass() override = default;

    void recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                        uint32_t frameIdx, uint32_t imageIdx) override;

    void initRenderpass() override;
};

#endif // RENDERING_RENDERPASSES_CLEARRENDERPASS_HPP
