#include "RenderingDevice.hpp"

#include "VulkanCommon.hpp"
#include "Rendering/VulkanPhysicalDevice.hpp"

RenderingDevice::RenderingDevice(VulkanPhysicalDevice *physicalDevice, VkDevice device,
                                 VkQueue graphicsQueue, VkQueue presentQueue)
        : _physicalDevice(physicalDevice),
          _device(device),
          _graphicsQueue(graphicsQueue),
          _presentQueue(presentQueue) {
    //
}

RenderingDevice::~RenderingDevice() {
    vkDestroyDevice(this->_device, nullptr);
}

void RenderingDevice::waitIdle() {
    vkEnsure(vkDeviceWaitIdle(this->_device));
}

VkDeviceMemory RenderingDevice::allocateMemory(VkMemoryRequirements requirements,
                                               VkMemoryPropertyFlags memoryProperty) {
    VkMemoryAllocateInfo allocateInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = nullptr,
            .allocationSize = requirements.size,
            .memoryTypeIndex = this->_physicalDevice->getSuitableMemoryType(requirements.memoryTypeBits,
                                                                            memoryProperty)
    };

    VkDeviceMemory memory;
    vkEnsure(vkAllocateMemory(this->_device, &allocateInfo, nullptr, &memory));

    return memory;
}

void *RenderingDevice::mapMemory(VkDeviceMemory memory, VkDeviceSize size) {
    void *ptr;
    vkEnsure(vkMapMemory(this->_device, memory, 0, size, 0, &ptr));

    return ptr;
}

void RenderingDevice::unmapMemory(VkDeviceMemory memory) {
    vkUnmapMemory(this->_device, memory);
}

void RenderingDevice::freeMemory(VkDeviceMemory memory) {
    vkFreeMemory(this->_device, memory, nullptr);
}

VkBuffer RenderingDevice::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage) {
    VkBufferCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .size = size,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr
    };

    VkBuffer buffer;
    vkEnsure(vkCreateBuffer(this->_device, &createInfo, nullptr, &buffer));

    return buffer;
}

VkMemoryRequirements RenderingDevice::getBufferMemoryRequirements(VkBuffer buffer) {
    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(this->_device, buffer, &requirements);

    return requirements;
}

void RenderingDevice::bindBufferMemory(VkBuffer buffer, VkDeviceMemory memory) {
    vkEnsure(vkBindBufferMemory(this->_device, buffer, memory, 0));
}

void RenderingDevice::destroyBuffer(VkBuffer buffer) {
    vkDestroyBuffer(this->_device, buffer, nullptr);
}
