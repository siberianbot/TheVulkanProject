#ifndef VULKANCOMMON_HPP
#define VULKANCOMMON_HPP

#include <vulkan/vulkan.hpp>

static constexpr void vkEnsure(VkResult vkExpression) {
    if (vkExpression != VK_SUCCESS) {
        throw std::runtime_error("Vulkan runtime error");
    }
}

static uint32_t vkGetSuitableMemoryType(VkPhysicalDevice physicalDevice,
                                        uint32_t memoryType, VkMemoryPropertyFlags memoryProperty) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    for (uint32_t idx = 0; idx < memoryProperties.memoryTypeCount; idx++) {
        bool memoryTypeMatches = memoryType & (1 << idx);
        bool memoryPropertyMatches =
                (memoryProperties.memoryTypes[idx].propertyFlags & memoryProperty) == memoryProperty;

        if (!memoryTypeMatches || !memoryPropertyMatches) {
            continue;
        }

        return idx;
    }

    throw std::runtime_error("not available");
}

static VkBuffer createBuffer(VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage) {
    VkBufferCreateInfo bufferCreateInfo = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .size = size,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr
    };

    VkBuffer buffer;
    vkEnsure(vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer));

    return buffer;
}

static VkDeviceMemory allocateMemoryForBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
                                              VkBuffer buffer, VkMemoryPropertyFlags memoryProperty) {
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

    uint32_t memoryType = vkGetSuitableMemoryType(physicalDevice,
                                                  memoryRequirements.memoryTypeBits, memoryProperty);

    VkMemoryAllocateInfo memoryAllocateInfo = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = nullptr,
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = memoryType
    };

    VkDeviceMemory memory;
    vkEnsure(vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &memory));
    vkEnsure(vkBindBufferMemory(device, buffer, memory, 0));

    return memory;
}

#endif // VULKANCOMMON_HPP
