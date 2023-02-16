#ifndef CLEARRENDERPASS_HPP
#define CLEARRENDERPASS_HPP

#include "RenderpassBase.hpp"

class ClearRenderpass : public RenderpassBase {
public:
    explicit ClearRenderpass(const DeviceData &deviceData);

    ~ClearRenderpass() override = default;

    void recordCommands(VkCommandBuffer commandBuffer, uint32_t framebufferIdx, VkRect2D renderArea) override;
};

#endif // CLEARRENDERPASS_HPP
