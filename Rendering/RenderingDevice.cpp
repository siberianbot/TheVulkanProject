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

VkImage RenderingDevice::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage,
                                     VkSampleCountFlagBits samples) {
    VkImageCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = format,
            .extent = {
                    .width = width,
                    .height = height,
                    .depth = 1
            },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = samples,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VkImage image;
    vkEnsure(vkCreateImage(this->_device, &createInfo, nullptr, &image));

    return image;
}

VkMemoryRequirements RenderingDevice::getImageMemoryRequirements(VkImage image) {
    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(this->_device, image, &requirements);

    return requirements;
}

void RenderingDevice::bindImageMemory(VkImage image, VkDeviceMemory memory) {
    vkEnsure(vkBindImageMemory(this->_device, image, memory, 0));
}

void RenderingDevice::destroyImage(VkImage image) {
    vkDestroyImage(this->_device, image, nullptr);
}

VkImageView RenderingDevice::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectMask) {
    VkImageViewCreateInfo imageViewCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = format,
            .components = {},
            .subresourceRange = {
                    .aspectMask = aspectMask,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
            }
    };

    VkImageView imageView;
    vkEnsure(vkCreateImageView(this->_device, &imageViewCreateInfo, nullptr, &imageView));

    return imageView;
}

void RenderingDevice::destroyImageView(VkImageView imageView) {
    vkDestroyImageView(this->_device, imageView, nullptr);
}
