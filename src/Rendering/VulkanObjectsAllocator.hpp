#ifndef RENDERING_VULKANOBJECTSALLOCATOR_HPP
#define RENDERING_VULKANOBJECTSALLOCATOR_HPP

#include <memory>

#include <vulkan/vulkan.hpp>

class PhysicalDevice;
class RenderingDevice;

class VulkanObjectsAllocator {
private:
    std::shared_ptr<PhysicalDevice> _physicalDevice;
    std::shared_ptr<RenderingDevice> _renderingDevice;

    uint32_t getSuitableMemoryTypeIdx(uint32_t memoryTypeBits, VkMemoryPropertyFlags requiredProperties);

    VkDeviceMemory allocateMemory(VkMemoryRequirements requirements, VkMemoryPropertyFlags properties);

public:
    VulkanObjectsAllocator(const std::shared_ptr<PhysicalDevice> &physicalDevice,
                           const std::shared_ptr<RenderingDevice> &renderingDevice);

    static std::shared_ptr<VulkanObjectsAllocator> create(const std::shared_ptr<PhysicalDevice> &physicalDevice,
                                                          const std::shared_ptr<RenderingDevice> &renderingDevice);

    VkCommandPool createCommandPool();
    void destroyCommandPool(VkCommandPool commandPool);

    VkBuffer createBuffer(VkBufferCreateInfo *createInfo);
    void destroyBuffer(VkBuffer buffer);

    VkImage createImage(VkImageCreateInfo *createInfo);
    void destroyImage(VkImage image);

    VkDeviceMemory allocateMemoryForBuffer(VkBuffer buffer, VkMemoryPropertyFlags properties);
    VkDeviceMemory allocateMemoryForImage(VkImage image, VkMemoryPropertyFlags properties);
    void freeMemory(VkDeviceMemory memory);
};

#endif // RENDERING_VULKANOBJECTSALLOCATOR_HPP
