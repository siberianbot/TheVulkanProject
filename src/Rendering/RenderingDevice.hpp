#ifndef RENDERING_RENDERINGDEVICE_HPP
#define RENDERING_RENDERINGDEVICE_HPP

#include <memory>
#include <optional>

#include <vulkan/vulkan.hpp>

class PhysicalDevice;

class RenderingDevice {
private:
    std::shared_ptr<PhysicalDevice> _physicalDevice;
    VkDevice _device;
    VkQueue _graphicsQueue;
    VkQueue _presentQueue;

public:
    RenderingDevice(const std::shared_ptr<PhysicalDevice> &physicalDevice, VkDevice device,
                    VkQueue graphicsQueue, VkQueue presentQueue);

    [[nodiscard]] VkDevice getHandle() const { return this->_device; }

    [[deprecated]]
    [[nodiscard]] std::shared_ptr<PhysicalDevice> getPhysicalDevice() const { return this->_physicalDevice; }

    [[nodiscard]] VkQueue getGraphicsQueue() const { return this->_graphicsQueue; }
    [[nodiscard]] VkQueue getPresentQueue() const { return this->_presentQueue; }

    void destroy();

#pragma region OBSOLETE

    VkSwapchainKHR createSwapchain(VkExtent2D extent, uint32_t minImageCount);
    std::vector<VkImage> getSwapchainImages(VkSwapchainKHR swapchain);
    void destroySwapchain(VkSwapchainKHR swapchain);

    VkFramebuffer createFramebuffer(VkRenderPass renderpass, VkExtent2D extent,
                                    const std::vector<VkImageView> &attachments);
    void destroyFramebuffer(VkFramebuffer framebuffer);

    VkSampler createSampler(const VkSamplerCreateInfo &createInfo);
    void destroySampler(VkSampler sampler);

    VkDescriptorPool createDescriptorPool(const std::vector<VkDescriptorPoolSize> &sizes, uint32_t maxSets);
    void destroyDescriptorPool(VkDescriptorPool descriptorPool);

    // TODO: should accept flags for any binding
    VkDescriptorSetLayout createDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding> &bindings);
    void destroyDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout);

    VkPipelineLayout createPipelineLayout(const std::vector<VkDescriptorSetLayout> &descriptorSetLayouts,
                                          const std::vector<VkPushConstantRange> &pushConstants);
    void destroyPipelineLayout(VkPipelineLayout pipelineLayout);

    VkRenderPass createRenderpass(const std::vector<VkAttachmentDescription> &attachments,
                                  const std::vector<VkSubpassDescription> &subpasses,
                                  const std::vector<VkSubpassDependency> &dependencies);
    void destroyRenderpass(VkRenderPass renderpass);

    VkPipeline createPipeline(const VkGraphicsPipelineCreateInfo *pipelineInfo);
    void destroyPipeline(VkPipeline pipeline);

#pragma endregion

    static std::shared_ptr<RenderingDevice> fromPhysicalDevice(const std::shared_ptr<PhysicalDevice> &physicalDevice);
};

#endif // RENDERING_RENDERINGDEVICE_HPP
