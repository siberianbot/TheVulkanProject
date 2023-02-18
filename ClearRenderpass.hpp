#ifndef CLEARRENDERPASS_HPP
#define CLEARRENDERPASS_HPP

#include "RenderpassBase.hpp"

class ClearRenderpass : public RenderpassBase {
public:
    explicit ClearRenderpass(RenderingDevice *renderingDevice);
    ~ClearRenderpass() override = default;

    void recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                        uint32_t frameIdx, uint32_t imageIdx) override;

    void initRenderpass() override;
};

#endif // CLEARRENDERPASS_HPP
