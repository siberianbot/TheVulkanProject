#ifndef RENDERING_RENDERPASSES_FINALRENDERPASS_HPP
#define RENDERING_RENDERPASSES_FINALRENDERPASS_HPP

#include "RenderpassBase.hpp"

class FinalRenderpass : public RenderpassBase {
public:
    explicit FinalRenderpass(RenderingDevice *renderingDevice, Swapchain *swapchain);
    ~FinalRenderpass() override = default;

    void recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                        uint32_t frameIdx, uint32_t imageIdx) override;

    void initRenderpass() override;
};

#endif // RENDERING_RENDERPASSES_FINALRENDERPASS_HPP
