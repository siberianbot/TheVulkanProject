#ifndef FINALRENDERPASS_HPP
#define FINALRENDERPASS_HPP

#include "RenderpassBase.hpp"

class FinalRenderpass : public RenderpassBase {
public:
    explicit FinalRenderpass(const RenderingDevice &renderingDevice);
    ~FinalRenderpass() override = default;

    void recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                        uint32_t frameIdx, uint32_t imageIdx) override;

    void initRenderpass() override;

    void createFramebuffers(uint32_t width, uint32_t height, uint32_t targetImagesCount,
                            const std::vector<std::vector<VkImageView>> &targetImageGroups) override;
};

#endif // FINALRENDERPASS_HPP
