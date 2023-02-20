#ifndef RENDERING_RENDERINGDEVICE_HPP
#define RENDERING_RENDERINGDEVICE_HPP

#include <optional>

#include <vulkan/vulkan.hpp>

class VulkanPhysicalDevice;

class RenderingDevice {
private:
    VulkanPhysicalDevice *_physicalDevice;
    VkDevice _device;
    VkQueue _graphicsQueue;
    VkQueue _presentQueue;

public:
    RenderingDevice(VulkanPhysicalDevice *physicalDevice, VkDevice device,
                    VkQueue graphicsQueue, VkQueue presentQueue);
    ~RenderingDevice();

    [[deprecated]] [[nodiscard]] VkDevice getHandle() const { return this->_device; }

    [[nodiscard]] VulkanPhysicalDevice *getPhysicalDevice() const { return this->_physicalDevice; }

    [[nodiscard]] VkQueue getGraphicsQueue() const { return this->_graphicsQueue; }
    [[nodiscard]] VkQueue getPresentQueue() const { return this->_presentQueue; }

    void waitIdle();

    VkDeviceMemory allocateMemory(VkMemoryRequirements requirements, VkMemoryPropertyFlags memoryProperty);
    void *mapMemory(VkDeviceMemory memory, VkDeviceSize size);
    void unmapMemory(VkDeviceMemory memory);
    void freeMemory(VkDeviceMemory memory);

    VkBuffer createBuffer(VkDeviceSize size, VkBufferUsageFlags usage);
    VkMemoryRequirements getBufferMemoryRequirements(VkBuffer buffer);
    void bindBufferMemory(VkBuffer buffer, VkDeviceMemory memory);
    void destroyBuffer(VkBuffer buffer);

    VkImage createImage(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage,
                        VkSampleCountFlagBits samples);
    VkMemoryRequirements getImageMemoryRequirements(VkImage image);
    void bindImageMemory(VkImage image, VkDeviceMemory memory);
    void destroyImage(VkImage image);

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectMask);
    void destroyImageView(VkImageView imageView);

    VkSwapchainKHR createSwapchain(VkExtent2D extent);
    std::vector<VkImage> getSwapchainImages(VkSwapchainKHR swapchain);
    std::optional<uint32_t> acquireNextSwapchainImageIdx(VkSwapchainKHR swapchain, uint64_t timeout,
                                                         VkSemaphore signalSemaphore);
    void destroySwapchain(VkSwapchainKHR swapchain);

    VkFramebuffer createFramebuffer(VkRenderPass renderpass, VkExtent2D extent, std::vector<VkImageView> attachments);
    void destroyFramebuffer(VkFramebuffer framebuffer);

    VkFence createFence(bool signaled);
    void waitForFence(VkFence fence, uint64_t timeout);
    void resetFence(VkFence fence);
    void destroyFence(VkFence fence);

    VkSemaphore createSemaphore();
    void destroySemaphore(VkSemaphore semaphore);
};

#endif // RENDERING_RENDERINGDEVICE_HPP
