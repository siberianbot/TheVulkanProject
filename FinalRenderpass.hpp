#ifndef FINALRENDERPASS_HPP
#define FINALRENDERPASS_HPP

#include "RenderpassBase.hpp"

class FinalRenderpass : public RenderpassBase {
public:
    explicit FinalRenderpass(RenderingDevice *renderingDevice);
    ~FinalRenderpass() override = default;

    void recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                        uint32_t frameIdx, uint32_t imageIdx) override;

    void initRenderpass() override;

    void createFramebuffers(const Swapchain &swapchain, const RenderTargets &renderTargets) override;
};

#endif // FINALRENDERPASS_HPP
