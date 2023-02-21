#include "RenderingDevice.hpp"

#include "Rendering/Common.hpp"
#include "Rendering/PhysicalDevice.hpp"

RenderingDevice::RenderingDevice(PhysicalDevice *physicalDevice, VkDevice device,
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

VkSwapchainKHR RenderingDevice::createSwapchain(VkExtent2D extent) {
    VkSurfaceFormatKHR surfaceFormat = this->_physicalDevice->getPreferredSurfaceFormat();
    VkPresentModeKHR presentMode = this->_physicalDevice->getPreferredPresentMode();
    VkSurfaceCapabilitiesKHR capabilities = this->_physicalDevice->getSurfaceCapabilities();

    uint32_t minImageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 &&
        minImageCount > capabilities.maxImageCount) {
        minImageCount = capabilities.maxImageCount;
    }

    bool exclusiveSharingMode = this->_physicalDevice->getGraphicsQueueFamilyIdx() ==
                                this->_physicalDevice->getPresentQueueFamilyIdx();

    uint32_t queueFamilies[] = {
            this->_physicalDevice->getGraphicsQueueFamilyIdx(),
            this->_physicalDevice->getPresentQueueFamilyIdx()
    };

    VkSwapchainCreateInfoKHR createInfo = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .surface = this->_physicalDevice->getSurface(),
            .minImageCount = minImageCount,
            .imageFormat = surfaceFormat.format,
            .imageColorSpace = surfaceFormat.colorSpace,
            .imageExtent = extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = exclusiveSharingMode
                                ? VK_SHARING_MODE_EXCLUSIVE
                                : VK_SHARING_MODE_CONCURRENT,
            .queueFamilyIndexCount = static_cast<uint32_t>(exclusiveSharingMode ? 0 : 2),
            .pQueueFamilyIndices = exclusiveSharingMode
                                   ? nullptr
                                   : queueFamilies,
            .preTransform = capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = presentMode,
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE
    };

    VkSwapchainKHR swapchain;
    vkEnsure(vkCreateSwapchainKHR(this->_device, &createInfo, nullptr, &swapchain));

    return swapchain;
}

std::vector<VkImage> RenderingDevice::getSwapchainImages(VkSwapchainKHR swapchain) {
    uint32_t count;
    vkEnsure(vkGetSwapchainImagesKHR(this->_device, swapchain, &count, nullptr));

    std::vector<VkImage> images(count);
    vkEnsure(vkGetSwapchainImagesKHR(this->_device, swapchain, &count, images.data()));

    return images;
}

void RenderingDevice::destroySwapchain(VkSwapchainKHR swapchain) {
    vkDestroySwapchainKHR(this->_device, swapchain, nullptr);
}

VkFramebuffer RenderingDevice::createFramebuffer(VkRenderPass renderpass, VkExtent2D extent,
                                                 std::vector<VkImageView> attachments) {
    const VkFramebufferCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .renderPass = renderpass,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .width = extent.width,
            .height = extent.height,
            .layers = 1
    };

    VkFramebuffer framebuffer;
    vkEnsure(vkCreateFramebuffer(this->_device, &createInfo, nullptr, &framebuffer));

    return framebuffer;
}

void RenderingDevice::destroyFramebuffer(VkFramebuffer framebuffer) {
    vkDestroyFramebuffer(this->_device, framebuffer, nullptr);
}

VkFence RenderingDevice::createFence(bool signaled) {
    VkFenceCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : (VkFenceCreateFlags) 0
    };

    VkFence fence;
    vkEnsure(vkCreateFence(this->_device, &createInfo, nullptr, &fence));

    return fence;
}

void RenderingDevice::waitForFence(VkFence fence, uint64_t timeout) {
    vkEnsure(vkWaitForFences(this->_device, 1, &fence, VK_TRUE, timeout));
}

void RenderingDevice::resetFence(VkFence fence) {
    vkEnsure(vkResetFences(this->_device, 1, &fence));
}

void RenderingDevice::destroyFence(VkFence fence) {
    vkDestroyFence(this->_device, fence, nullptr);
}

VkSemaphore RenderingDevice::createSemaphore() {
    VkSemaphoreCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0
    };

    VkSemaphore semaphore;
    vkEnsure(vkCreateSemaphore(this->_device, &createInfo, nullptr, &semaphore));

    return semaphore;
}

void RenderingDevice::destroySemaphore(VkSemaphore semaphore) {
    vkDestroySemaphore(this->_device, semaphore, nullptr);
}

std::optional<uint32_t> RenderingDevice::acquireNextSwapchainImageIdx(VkSwapchainKHR swapchain, uint64_t timeout,
                                                                      VkSemaphore signalSemaphore) {
    uint32_t imageIdx;
    VkResult result = vkAcquireNextImageKHR(this->_device, swapchain, timeout, signalSemaphore,
                                            VK_NULL_HANDLE, &imageIdx);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        return std::nullopt;
    } else if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR) {
        return imageIdx;
    }

    throw std::runtime_error("Vulkan runtime error");
}

VkSampler RenderingDevice::createSampler() {
    VkSamplerCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .mipLodBias = 0,
            .anisotropyEnable = VK_TRUE,
            .maxAnisotropy = this->_physicalDevice->getMaxSamplerAnisotropy(),
            .compareEnable = VK_FALSE,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .minLod = 0,
            .maxLod = 1,
            .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE
    };

    VkSampler sampler;
    vkEnsure(vkCreateSampler(this->_device, &createInfo, nullptr, &sampler));

    return sampler;
}

void RenderingDevice::destroySampler(VkSampler sampler) {
    vkDestroySampler(this->_device, sampler, nullptr);
}

VkDescriptorPool RenderingDevice::createDescriptorPool(const std::vector<VkDescriptorPoolSize> sizes,
                                                       uint32_t maxSets) {
    VkDescriptorPoolCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
            .maxSets = maxSets,
            .poolSizeCount = static_cast<uint32_t>(sizes.size()),
            .pPoolSizes = sizes.data()
    };

    VkDescriptorPool descriptorPool;
    vkEnsure(vkCreateDescriptorPool(this->_device, &createInfo, nullptr, &descriptorPool));

    return descriptorPool;
}

void RenderingDevice::destroyDescriptorPool(VkDescriptorPool descriptorPool) {
    vkDestroyDescriptorPool(this->_device, descriptorPool, nullptr);
}

VkDescriptorSetLayout RenderingDevice::createDescriptorSetLayout(
        const std::vector<VkDescriptorSetLayoutBinding> bindings) {
    VkDescriptorSetLayoutCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .bindingCount = static_cast<uint32_t>(bindings.size()),
            .pBindings = bindings.data()
    };

    VkDescriptorSetLayout descriptorSetLayout;
    vkEnsure(vkCreateDescriptorSetLayout(this->_device, &createInfo, nullptr, &descriptorSetLayout));

    return descriptorSetLayout;
}

void RenderingDevice::destroyDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) {
    vkDestroyDescriptorSetLayout(this->_device, descriptorSetLayout, nullptr);
}

VkPipelineLayout RenderingDevice::createPipelineLayout(const std::vector<VkDescriptorSetLayout> descriptorSetLayouts,
                                                       const std::vector<VkPushConstantRange> pushConstants) {
    VkPipelineLayoutCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
            .pSetLayouts = descriptorSetLayouts.data(),
            .pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size()),
            .pPushConstantRanges = pushConstants.data()
    };

    VkPipelineLayout pipelineLayout;
    vkEnsure(vkCreatePipelineLayout(this->_device, &createInfo, nullptr, &pipelineLayout));

    return pipelineLayout;
}

void RenderingDevice::destroyPipelineLayout(VkPipelineLayout pipelineLayout) {
    vkDestroyPipelineLayout(this->_device, pipelineLayout, nullptr);
}

std::array<VkDescriptorSet, MAX_INFLIGHT_FRAMES> RenderingDevice::allocateDescriptorSets(
        VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout) {
    std::vector<VkDescriptorSetLayout> layouts(MAX_INFLIGHT_FRAMES, descriptorSetLayout);
    std::array<VkDescriptorSet, MAX_INFLIGHT_FRAMES> descriptorSets = {};

    VkDescriptorSetAllocateInfo allocateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = descriptorPool,
            .descriptorSetCount = static_cast<uint32_t>(descriptorSets.size()),
            .pSetLayouts = layouts.data()
    };

    vkEnsure(vkAllocateDescriptorSets(this->_device, &allocateInfo, descriptorSets.data()));

    return descriptorSets;
}

void RenderingDevice::freeDescriptorSets(VkDescriptorPool descriptorPool,
                                         const std::array<VkDescriptorSet, MAX_INFLIGHT_FRAMES> &descriptorSets) {
    vkEnsure(vkFreeDescriptorSets(this->_device, descriptorPool, static_cast<uint32_t>(descriptorSets.size()),
                                  descriptorSets.data()));
}

void RenderingDevice::updateDescriptorSets(std::vector<VkWriteDescriptorSet> writes) {
    vkUpdateDescriptorSets(this->_device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}
