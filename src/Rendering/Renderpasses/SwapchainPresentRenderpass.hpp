#ifndef RENDERING_RENDERPASSES_SWAPCHAINPRESENTRENDERPASS_HPP
#define RENDERING_RENDERPASSES_SWAPCHAINPRESENTRENDERPASS_HPP

#include <vector>

#include "RenderpassBase.hpp"

class Swapchain;
class DescriptorSetObject;

class SwapchainPresentRenderpass : public RenderpassBase {
private:
    Swapchain *_swapchain;

    std::vector<RenderpassBase *> _inputRenderpasses;

    VkDescriptorPool _descriptorPool;
    VkDescriptorSetLayout _descriptorSetLayout;
    VkPipelineLayout _pipelineLayout;
    VkPipeline _pipeline;
    DescriptorSetObject *_descriptorSet;

public:
    SwapchainPresentRenderpass(RenderingDevice *renderingDevice, Swapchain *swapchain);
    ~SwapchainPresentRenderpass() override = default;

    void recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                        uint32_t frameIdx, uint32_t imageIdx) override;

    void addInputRenderpass(RenderpassBase *renderpass);

    void initRenderpass() override;
    void destroyRenderpass() override;

    void createFramebuffers() override;
    void destroyFramebuffers() override;

    ImageViewObject *getResultImageView(uint32_t imageIdx) override { return nullptr; }
};

#endif // RENDERING_RENDERPASSES_SWAPCHAINPRESENTRENDERPASS_HPP
