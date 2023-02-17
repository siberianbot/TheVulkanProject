#ifndef CLEARRENDERPASS_HPP
#define CLEARRENDERPASS_HPP

#include "RenderpassBase.hpp"

class ClearRenderpass : public RenderpassBase {
public:
    explicit ClearRenderpass(const DeviceData &deviceData);

    ~ClearRenderpass() override = default;

    void
    recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea, uint32_t frameIdx, uint32_t imageIdx) override;
};

#endif // CLEARRENDERPASS_HPP
