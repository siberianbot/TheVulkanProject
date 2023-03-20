#include "VulkanObjectsAllocator.hpp"

#include "src/Rendering/Common.hpp"
#include "src/Rendering/PhysicalDevice.hpp"
#include "src/Rendering/RenderingDevice.hpp"

uint32_t VulkanObjectsAllocator::getSuitableMemoryTypeIdx(uint32_t memoryTypeBits,
                                                          VkMemoryPropertyFlags requiredProperties) {
    VkPhysicalDeviceMemoryProperties properties;
    vkGetPhysicalDeviceMemoryProperties(this->_physicalDevice->getHandle(), &properties);

    for (uint32_t idx = 0; idx < properties.memoryTypeCount; idx++) {
        bool typeMatches = memoryTypeBits & (1 << idx);
        bool propertiesMatches = (properties.memoryTypes[idx].propertyFlags & requiredProperties) == requiredProperties;

        if (!typeMatches || !propertiesMatches) {
            continue;
        }

        return idx;
    }

    throw std::runtime_error("No suitable memory type available");
}

VkDeviceMemory VulkanObjectsAllocator::allocateMemory(VkMemoryRequirements requirements,
                                                      VkMemoryPropertyFlags properties) {
    VkMemoryAllocateInfo allocateInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = nullptr,
            .allocationSize = requirements.size,
            .memoryTypeIndex = this->getSuitableMemoryTypeIdx(requirements.memoryTypeBits, properties)
    };

    VkDeviceMemory memory;
    vkEnsure(vkAllocateMemory(this->_renderingDevice->getHandle(), &allocateInfo, nullptr, &memory));

    return memory;
}

VulkanObjectsAllocator::VulkanObjectsAllocator(const std::shared_ptr<PhysicalDevice> &physicalDevice,
                                               const std::shared_ptr<RenderingDevice> &renderingDevice)
        : _physicalDevice(physicalDevice),
          _renderingDevice(renderingDevice) {
    //
}

std::shared_ptr<VulkanObjectsAllocator>
VulkanObjectsAllocator::create(const std::shared_ptr<PhysicalDevice> &physicalDevice,
                               const std::shared_ptr<RenderingDevice> &renderingDevice) {
    return std::make_shared<VulkanObjectsAllocator>(physicalDevice, renderingDevice);
}

VkCommandPool VulkanObjectsAllocator::createCommandPool() {
    VkCommandPoolCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = this->_physicalDevice->getGraphicsQueueFamilyIdx()
    };

    VkCommandPool commandPool;
    vkEnsure(vkCreateCommandPool(this->_renderingDevice->getHandle(), &createInfo, nullptr, &commandPool));

    return commandPool;
}

void VulkanObjectsAllocator::destroyCommandPool(VkCommandPool commandPool) {
    vkDestroyCommandPool(this->_renderingDevice->getHandle(), commandPool, nullptr);
}

VkBuffer VulkanObjectsAllocator::createBuffer(VkBufferCreateInfo *createInfo) {
    VkBuffer buffer;
    vkEnsure(vkCreateBuffer(this->_renderingDevice->getHandle(), createInfo, nullptr, &buffer));

    return buffer;
}

void VulkanObjectsAllocator::destroyBuffer(VkBuffer buffer) {
    vkDestroyBuffer(this->_renderingDevice->getHandle(), buffer, nullptr);
}

VkImage VulkanObjectsAllocator::createImage(VkImageCreateInfo *createInfo) {
    VkImage image;
    vkEnsure(vkCreateImage(this->_renderingDevice->getHandle(), createInfo, nullptr, &image));

    return image;
}

void VulkanObjectsAllocator::destroyImage(VkImage image) {
    vkDestroyImage(this->_renderingDevice->getHandle(), image, nullptr);
}

VkDeviceMemory VulkanObjectsAllocator::allocateMemoryForBuffer(VkBuffer buffer, VkMemoryPropertyFlags properties) {
    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(this->_renderingDevice->getHandle(), buffer, &requirements);

    return this->allocateMemory(requirements, properties);
}

VkDeviceMemory VulkanObjectsAllocator::allocateMemoryForImage(VkImage image, VkMemoryPropertyFlags properties) {
    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(this->_renderingDevice->getHandle(), image, &requirements);

    return this->allocateMemory(requirements, properties);
}

void VulkanObjectsAllocator::freeMemory(VkDeviceMemory memory) {
    vkFreeMemory(this->_renderingDevice->getHandle(), memory, nullptr);
}
