#ifndef RENDERING_RENDERPASSES_IMGUIRENDERPASS_HPP
#define RENDERING_RENDERPASSES_IMGUIRENDERPASS_HPP

#include "RenderpassBase.hpp"

class PhysicalDevice;
class CommandExecutor;
class Swapchain;
class RenderingObjectsFactory;
class ImageObject;
class ImageViewObject;

class ImguiRenderpass : public RenderpassBase {
private:
    VkInstance _instance;
    PhysicalDevice *_physicalDevice;
    CommandExecutor *_commandExecutor;
    Swapchain *_swapchain;
    RenderingObjectsFactory *_renderingObjectsFactory;
    VkDescriptorPool _descriptorPool;

    ImageObject *_resultImage;
    ImageViewObject *_resultImageView;

public:
    ImguiRenderpass(RenderingDevice *renderingDevice, Swapchain *swapchain,
                    RenderingObjectsFactory *renderingObjectsFactory, VkInstance instance,
                    PhysicalDevice *physicalDevice, CommandExecutor *commandExecutor);
    ~ImguiRenderpass() override = default;

    void recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                        uint32_t frameIdx, uint32_t imageIdx) override;

    void initRenderpass() override;
    void destroyRenderpass() override;

    void createFramebuffers() override;
    void destroyFramebuffers() override;

    ImageViewObject *getResultImageView(uint32_t imageIdx) override { return this->_resultImageView; }
};

#endif // RENDERING_RENDERPASSES_IMGUIRENDERPASS_HPP
