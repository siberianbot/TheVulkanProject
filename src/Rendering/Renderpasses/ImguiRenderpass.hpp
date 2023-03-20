#ifndef RENDERING_RENDERPASSES_IMGUIRENDERPASS_HPP
#define RENDERING_RENDERPASSES_IMGUIRENDERPASS_HPP

#include "RenderpassBase.hpp"

class PhysicalDevice;
class CommandExecutor;
class Swapchain;
class VulkanObjectsAllocator;
class ImageObject;
class ImageViewObject;

class ImguiRenderpass : public RenderpassBase {
private:
    std::shared_ptr<VulkanObjectsAllocator> _vulkanObjectsAllocator;

    VkInstance _instance;
    PhysicalDevice *_physicalDevice;
    CommandExecutor *_commandExecutor;
    Swapchain *_swapchain;
    VkDescriptorPool _descriptorPool;

    std::shared_ptr<ImageObject> _resultImage;
    std::shared_ptr<ImageViewObject> _resultImageView;

public:
    ImguiRenderpass(const std::shared_ptr<RenderingDevice> &renderingDevice,
                    const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator,
                    Swapchain *swapchain, VkInstance instance,
                    PhysicalDevice *physicalDevice, CommandExecutor *commandExecutor);
    ~ImguiRenderpass() override = default;

    void recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                        uint32_t frameIdx, uint32_t imageIdx) override;

    void initRenderpass() override;
    void destroyRenderpass() override;

    void createFramebuffers() override;
    void destroyFramebuffers() override;

    std::shared_ptr<ImageViewObject> getResultImageView(uint32_t imageIdx) override { return this->_resultImageView; }
};

#endif // RENDERING_RENDERPASSES_IMGUIRENDERPASS_HPP
