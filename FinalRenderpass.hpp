#ifndef FINALRENDERPASS_HPP
#define FINALRENDERPASS_HPP

#include "RenderpassBase.hpp"

class FinalRenderpass : public RenderpassBase {
public:
    explicit FinalRenderpass(const DeviceData &deviceData);

    ~FinalRenderpass() override = default;

    void recordCommands(VkCommandBuffer commandBuffer, uint32_t framebufferIdx, VkRect2D renderArea) override;

    void createRenderpass() override;
    void createFramebuffers(uint32_t width, uint32_t height, uint32_t targetImagesCount,
                            const std::vector<std::vector<VkImageView>> &targetImageGroups) override;
};

#endif // FINALRENDERPASS_HPP
