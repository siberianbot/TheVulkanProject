#include "RenderingDevice.hpp"

#include <set>

#include "Common.hpp"
#include "PhysicalDevice.hpp"

RenderingDevice::RenderingDevice(const std::shared_ptr<PhysicalDevice> &physicalDevice, VkDevice device,
                                 VkQueue graphicsQueue, VkQueue presentQueue)
        : _physicalDevice(physicalDevice),
          _device(device),
          _graphicsQueue(graphicsQueue),
          _presentQueue(presentQueue) {
    //
}

void RenderingDevice::destroy() {
    vkDestroyDevice(this->_device, nullptr);
}

void RenderingDevice::waitIdle() {
    vkEnsure(vkDeviceWaitIdle(this->_device));
}

VkSwapchainKHR RenderingDevice::createSwapchain(VkExtent2D extent, uint32_t minImageCount) {
    VkSurfaceFormatKHR surfaceFormat = this->_physicalDevice->getPreferredSurfaceFormat();
    VkPresentModeKHR presentMode = this->_physicalDevice->getPreferredPresentMode();
    VkSurfaceCapabilitiesKHR capabilities = this->_physicalDevice->getSurfaceCapabilities();

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
                                                 const std::vector<VkImageView> &attachments) {
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

VkSampler RenderingDevice::createSampler(const VkSamplerCreateInfo &createInfo) {
    VkSampler sampler;
    vkEnsure(vkCreateSampler(this->_device, &createInfo, nullptr, &sampler));

    return sampler;
}

void RenderingDevice::destroySampler(VkSampler sampler) {
    vkDestroySampler(this->_device, sampler, nullptr);
}

VkDescriptorPool RenderingDevice::createDescriptorPool(const std::vector<VkDescriptorPoolSize> &sizes,
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
        const std::vector<VkDescriptorSetLayoutBinding> &bindings) {
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

VkPipelineLayout RenderingDevice::createPipelineLayout(const std::vector<VkDescriptorSetLayout> &descriptorSetLayouts,
                                                       const std::vector<VkPushConstantRange> &pushConstants) {
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

std::vector<VkDescriptorSet> RenderingDevice::allocateDescriptorSets(uint32_t count, VkDescriptorPool descriptorPool,
                                                                     VkDescriptorSetLayout descriptorSetLayout) {
    std::vector<VkDescriptorSetLayout> layouts(count, descriptorSetLayout);
    std::vector<VkDescriptorSet> descriptorSets(count);

    VkDescriptorSetAllocateInfo allocateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = descriptorPool,
            .descriptorSetCount = count,
            .pSetLayouts = layouts.data()
    };

    vkEnsure(vkAllocateDescriptorSets(this->_device, &allocateInfo, descriptorSets.data()));

    return descriptorSets;
}

void RenderingDevice::freeDescriptorSets(VkDescriptorPool descriptorPool, uint32_t count, const VkDescriptorSet *ptr) {
    vkEnsure(vkFreeDescriptorSets(this->_device, descriptorPool, count, ptr));
}

void RenderingDevice::updateDescriptorSets(const std::vector<VkWriteDescriptorSet> &writes) {
    vkUpdateDescriptorSets(this->_device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

VkRenderPass RenderingDevice::createRenderpass(const std::vector<VkAttachmentDescription> &attachments,
                                               const std::vector<VkSubpassDescription> &subpasses,
                                               const std::vector<VkSubpassDependency> &dependencies) {
    VkRenderPassCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .subpassCount = static_cast<uint32_t>(subpasses.size()),
            .pSubpasses = subpasses.data(),
            .dependencyCount = static_cast<uint32_t>(dependencies.size()),
            .pDependencies = dependencies.data()
    };

    VkRenderPass renderpass;
    vkEnsure(vkCreateRenderPass(this->_device, &createInfo, nullptr, &renderpass));

    return renderpass;
}

void RenderingDevice::destroyRenderpass(VkRenderPass renderpass) {
    vkDestroyRenderPass(this->_device, renderpass, nullptr);
}

VkPipeline RenderingDevice::createPipeline(const VkGraphicsPipelineCreateInfo *pipelineInfo) {
    VkPipeline pipeline;
    vkEnsure(vkCreateGraphicsPipelines(this->_device, VK_NULL_HANDLE, 1, pipelineInfo, nullptr, &pipeline));

    return pipeline;
}

void RenderingDevice::destroyPipeline(VkPipeline pipeline) {
    vkDestroyPipeline(this->_device, pipeline, nullptr);
}

std::shared_ptr<RenderingDevice> RenderingDevice::fromPhysicalDevice(
        const std::shared_ptr<PhysicalDevice> &physicalDevice) {
    const float queuePriority = 1.0f;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> familyIndices = {
            physicalDevice->getGraphicsQueueFamilyIdx(),
            physicalDevice->getPresentQueueFamilyIdx()
    };

    for (uint32_t familyIdx: familyIndices) {
        VkDeviceQueueCreateInfo queueCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueFamilyIndex = familyIdx,
                .queueCount = 1,
                .pQueuePriorities = &queuePriority
        };

        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures physicalDeviceFeatures = {
            .samplerAnisotropy = VK_TRUE
    };

    VkDeviceCreateInfo deviceCreateInfo = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size()),
            .ppEnabledLayerNames = VALIDATION_LAYERS.data(),
            .enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size()),
            .ppEnabledExtensionNames = DEVICE_EXTENSIONS.data(),
            .pEnabledFeatures = &physicalDeviceFeatures
    };

    VkDevice device;
    vkEnsure(vkCreateDevice(physicalDevice->getHandle(), &deviceCreateInfo, nullptr, &device));

    VkQueue graphicsQueue, presentQueue;
    vkGetDeviceQueue(device, physicalDevice->getGraphicsQueueFamilyIdx(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, physicalDevice->getPresentQueueFamilyIdx(), 0, &presentQueue);

    return std::make_shared<RenderingDevice>(physicalDevice, device, graphicsQueue, presentQueue);
}
