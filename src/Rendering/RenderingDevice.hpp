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
    void waitIdle();

#pragma region OBSOLETE

    VkImageView createImageView(VkImage image, uint32_t layers, VkImageViewType imageViewType,
                                VkFormat format, VkImageAspectFlags aspectMask);
    void destroyImageView(VkImageView imageView);

    VkSwapchainKHR createSwapchain(VkExtent2D extent, uint32_t minImageCount);
    std::vector<VkImage> getSwapchainImages(VkSwapchainKHR swapchain);
    std::optional<uint32_t> acquireNextSwapchainImageIdx(VkSwapchainKHR swapchain, uint64_t timeout,
                                                         VkSemaphore signalSemaphore);
    void destroySwapchain(VkSwapchainKHR swapchain);

    VkFramebuffer createFramebuffer(VkRenderPass renderpass, VkExtent2D extent,
                                    const std::vector<VkImageView> &attachments);
    void destroyFramebuffer(VkFramebuffer framebuffer);

    VkFence createFence(bool signaled);
    void waitForFence(VkFence fence, uint64_t timeout);
    void resetFence(VkFence fence);
    void destroyFence(VkFence fence);

    VkSemaphore createSemaphore();
    void destroySemaphore(VkSemaphore semaphore);

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

    std::vector<VkDescriptorSet> allocateDescriptorSets(uint32_t count, VkDescriptorPool descriptorPool,
                                                        VkDescriptorSetLayout descriptorSetLayout);
    void freeDescriptorSets(VkDescriptorPool descriptorPool, uint32_t count, const VkDescriptorSet *ptr);
    void updateDescriptorSets(const std::vector<VkWriteDescriptorSet> &writes);

    VkRenderPass createRenderpass(const std::vector<VkAttachmentDescription> &attachments,
                                  const std::vector<VkSubpassDescription> &subpasses,
                                  const std::vector<VkSubpassDependency> &dependencies);
    void destroyRenderpass(VkRenderPass renderpass);

    VkShaderModule createShaderModule(const std::vector<char> &content);
    void destroyShaderModule(VkShaderModule shaderModule);

    VkPipeline createPipeline(const VkGraphicsPipelineCreateInfo *pipelineInfo);
    void destroyPipeline(VkPipeline pipeline);

#pragma endregion

    static std::shared_ptr<RenderingDevice> fromPhysicalDevice(const std::shared_ptr<PhysicalDevice> &physicalDevice);
};

#endif // RENDERING_RENDERINGDEVICE_HPP
