#ifndef RENDERING_RENDERPASSES_IMGUIRENDERPASS_HPP
#define RENDERING_RENDERPASSES_IMGUIRENDERPASS_HPP

#include "RenderpassBase.hpp"

class PhysicalDevice;
class CommandExecutor;

class ImguiRenderpass : public RenderpassBase {
private:
    VkInstance _instance;
    PhysicalDevice *_physicalDevice;
    CommandExecutor *_commandExecutor;
    VkDescriptorPool _descriptorPool;

public:
    ImguiRenderpass(RenderingDevice *renderingDevice, Swapchain *swapchain,
                    VkInstance instance, PhysicalDevice *physicalDevice, CommandExecutor *commandExecutor);
    ~ImguiRenderpass() override = default;

    void recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                        uint32_t frameIdx, uint32_t imageIdx) override;

    void initRenderpass() override;
    void destroyRenderpass() override;

    void createFramebuffers() override;
};

#endif // RENDERING_RENDERPASSES_IMGUIRENDERPASS_HPP
