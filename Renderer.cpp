#include "Renderer.hpp"

#include <set>
#include <iostream>
#include <array>
#include <limits>

#include "Constants.hpp"
#include "Engine.hpp"
#include "ClearRenderpass.hpp"
#include "FinalRenderpass.hpp"
#include "VulkanCommon.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

VkExtent2D getPreferredExtent(VkSurfaceCapabilitiesKHR capabilities, VkExtent2D currentExtent) {
    const uint32_t uint32max = std::numeric_limits<uint32_t>::max();

    if (capabilities.currentExtent.width != uint32max && capabilities.currentExtent.height != uint32max) {
        return capabilities.currentExtent;
    }

    currentExtent.width = std::clamp(currentExtent.width,
                                     capabilities.minImageExtent.width,
                                     capabilities.maxImageExtent.width);

    currentExtent.height = std::clamp(currentExtent.height,
                                      capabilities.minImageExtent.height,
                                      capabilities.maxImageExtent.height);

    return currentExtent;
}

std::vector<const char *> vkRequiredExtensions() {
    uint32_t count;
    const char **extensions = glfwGetRequiredInstanceExtensions(&count);

    std::vector<const char *> result(extensions, extensions + count);
    result.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return result;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                      VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                      const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                      void *pUserData) {
    const char *type;
    switch (messageType) {
        case VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
            type = "vk validation";
            break;

        case VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
            type = "vk performance";
            break;

        case VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT:
            type = "vk binding";
            break;

        case VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
            type = "vk general";
            break;

        default:
            throw std::runtime_error("not supported");
    }

    if (messageSeverity == VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        std::cerr << type << ": " << pCallbackData->pMessage << std::endl;
    } else {
        std::cout << type << ": " << pCallbackData->pMessage << std::endl;
    }

    return VK_FALSE;
}

VkImageView Renderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectMask) {
    VkImageViewCreateInfo imageViewCreateInfo = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = image,
            .viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D,
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
    vkEnsure(vkCreateImageView(this->device, &imageViewCreateInfo, nullptr, &imageView));

    return imageView;
}

VkImage Renderer::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage,
                              VkSampleCountFlagBits samples) {
    VkImageCreateInfo imageCreateInfo = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
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
            .tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VkImage image;
    vkEnsure(vkCreateImage(this->device, &imageCreateInfo, nullptr, &image));

    return image;
}

VkDeviceMemory Renderer::allocateMemoryForImage(VkImage image, VkMemoryPropertyFlags memoryProperty) {
    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(this->device, image, &memoryRequirements);

    uint32_t memoryType = this->_physicalDevice->getSuitableMemoryType(memoryRequirements.memoryTypeBits,
                                                                       memoryProperty);

    VkMemoryAllocateInfo memoryAllocateInfo = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = nullptr,
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = memoryType
    };

    VkDeviceMemory memory;
    vkEnsure(vkAllocateMemory(this->device, &memoryAllocateInfo, nullptr, &memory));
    vkEnsure(vkBindImageMemory(this->device, image, memory, 0));

    return memory;
}

Renderer::Renderer(Engine *engine) : engine(engine) {
    //
}

void Renderer::init() {
    initInstance();
    initSurface(this->engine->window());

    this->_physicalDevice = VulkanPhysicalDevice::selectSuitable(this->instance, this->surface);

    initDevice();

    initSwapchain();
    initSwapchainResources();

    RenderingDevice renderingDevice = {
            .device = this->device,
            .colorFormat = this->_physicalDevice->getColorFormat(),
            .depthFormat = this->_physicalDevice->getDepthFormat(),
            .samples = this->_physicalDevice->getMsaaSamples(),
            .graphicsQueue = this->graphicsQueue,
            .graphicsQueueFamilyIdx = this->_physicalDevice->getGraphicsQueueFamilyIdx()
    };

    this->_vulkanCommandExecutor = new VulkanCommandExecutor(renderingDevice);

    initSync();

    initUniformBuffers();
    initTextureSampler();
    initDescriptors();
    initLayouts();

    this->_renderpasses.push_back(new ClearRenderpass(renderingDevice));
    this->_sceneRenderpass = new SceneRenderpass(renderingDevice, this->pipelineLayout);
    this->_renderpasses.push_back(this->_sceneRenderpass);
    this->_renderpasses.push_back(new FinalRenderpass(renderingDevice));

    uint32_t swapchainImagesCount = this->swapchainImageViews.size();
    Swapchain swapchain_ = {
            .width = this->swapchainExtent.width,
            .height = this->swapchainExtent.height,
            .swapchainImagesCount = swapchainImagesCount
    };

    RenderTargets renderTargets = {
            .colorGroup = std::vector<VkImageView>(swapchainImagesCount, this->colorImageView),
            .depthGroup = std::vector<VkImageView>(swapchainImagesCount, this->depthImageView),
            .resolveGroup = this->swapchainImageViews
    };

    for (RenderpassBase *renderpass: this->_renderpasses) {
        renderpass->initRenderpass();
        renderpass->createFramebuffers(swapchain_, renderTargets);
    }
}

void Renderer::cleanup() {
    vkEnsure(vkDeviceWaitIdle(this->device));

    cleanupSwapchain();

    this->_sceneRenderpass = nullptr;
    for (RenderpassBase *renderpass: this->_renderpasses) {
        renderpass->destroyRenderpass();
        delete renderpass;
    }

    vkDestroyPipelineLayout(this->device, this->pipelineLayout, nullptr);

    vkDestroyDescriptorSetLayout(this->device, this->descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(this->device, this->descriptorPool, nullptr);

    vkDestroySampler(this->device, this->textureSampler, nullptr);

    for (int idx = 0; idx < VK_MAX_INFLIGHT_FRAMES; idx++) {
        vkDestroySemaphore(this->device, this->renderFinishedSemaphores[idx], nullptr);
        vkDestroySemaphore(this->device, this->imageAvailableSemaphores[idx], nullptr);
        vkDestroyFence(this->device, this->fences[idx], nullptr);

        vkUnmapMemory(this->device, this->uniformBufferMemory[idx]);
        vkFreeMemory(this->device, this->uniformBufferMemory[idx], nullptr);
        vkDestroyBuffer(this->device, this->uniformBuffers[idx], nullptr);
    }

    delete this->_vulkanCommandExecutor;

    vkDestroyDevice(this->device, nullptr);
    vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
    vkDestroyInstance(this->instance, nullptr);
}

void Renderer::requestResize(uint32_t width, uint32_t height) {
    this->currentExtent = {width, height};
    this->resizeRequested = true;
}

void Renderer::initInstance() {
    auto extensions = vkRequiredExtensions();
    uint32_t version = VK_MAKE_VERSION(0, 1, 0);

    VkApplicationInfo appInfo = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = NAME,
            .applicationVersion = version,
            .pEngineName = NAME,
            .engineVersion = version,
            .apiVersion = VK_API_VERSION_1_1,
    };

    VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoExt = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext = nullptr,
            .flags = 0,
            .messageSeverity = VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                               VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                               VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
            .messageType = VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT |
                           VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                           VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
            .pfnUserCallback = vkDebugCallback,
            .pUserData = this
    };

    VkInstanceCreateInfo instanceCreateInfo = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = &debugUtilsMessengerCreateInfoExt,
            .flags = 0,
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = static_cast<uint32_t>(VK_VALIDATION_LAYERS.size()),
            .ppEnabledLayerNames = VK_VALIDATION_LAYERS.data(),
            .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data()
    };

    vkEnsure(vkCreateInstance(&instanceCreateInfo, nullptr, &this->instance));
}

void Renderer::initSurface(GLFWwindow *window) {
    int w, h;
    glfwGetWindowSize(window, &w, &h);
    currentExtent = {(uint32_t) w, (uint32_t) h};

    vkEnsure(glfwCreateWindowSurface(this->instance, window, nullptr, &this->surface));
}

void Renderer::initDevice() {
    const float queuePriority = 1.0f;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> familyIndices = {
            this->_physicalDevice->getGraphicsQueueFamilyIdx(),
            this->_physicalDevice->getPresentQueueFamilyIdx()
    };

    for (uint32_t familyIdx: familyIndices) {
        VkDeviceQueueCreateInfo queueCreateInfo = {
                .sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
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
            .enabledLayerCount = static_cast<uint32_t>(VK_VALIDATION_LAYERS.size()),
            .ppEnabledLayerNames = VK_VALIDATION_LAYERS.data(),
            .enabledExtensionCount = static_cast<uint32_t>(VK_DEVICE_EXTENSIONS.size()),
            .ppEnabledExtensionNames = VK_DEVICE_EXTENSIONS.data(),
            .pEnabledFeatures = &physicalDeviceFeatures
    };

    vkEnsure(vkCreateDevice(this->_physicalDevice->getHandle(), &deviceCreateInfo, nullptr, &this->device));
    vkGetDeviceQueue(this->device, this->_physicalDevice->getGraphicsQueueFamilyIdx(), 0, &this->graphicsQueue);
    vkGetDeviceQueue(this->device, this->_physicalDevice->getPresentQueueFamilyIdx(), 0, &this->presentQueue);
}

void Renderer::initSwapchain() {
    VkSurfaceFormatKHR surfaceFormat = this->_physicalDevice->getPreferredSurfaceFormat();
    VkPresentModeKHR presentMode = this->_physicalDevice->getPreferredPresentMode();
    VkSurfaceCapabilitiesKHR capabilities = this->_physicalDevice->getSurfaceCapabilities();

    VkExtent2D extent = getPreferredExtent(capabilities, this->currentExtent);

    uint32_t minImageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 &&
        minImageCount > capabilities.maxImageCount) {
        minImageCount = capabilities.maxImageCount;
    }

    bool exclusiveSharingMode = this->_physicalDevice->getGraphicsQueueFamilyIdx() ==
                                this->_physicalDevice->getPresentQueueFamilyIdx();

    uint32_t queueFamilyIndices[] = {
            this->_physicalDevice->getGraphicsQueueFamilyIdx(),
            this->_physicalDevice->getPresentQueueFamilyIdx()
    };

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .surface = this->surface,
            .minImageCount = minImageCount,
            .imageFormat = surfaceFormat.format,
            .imageColorSpace = surfaceFormat.colorSpace,
            .imageExtent = extent,
            .imageArrayLayers = 1,
            .imageUsage = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = exclusiveSharingMode
                                ? VkSharingMode::VK_SHARING_MODE_EXCLUSIVE
                                : VkSharingMode::VK_SHARING_MODE_CONCURRENT,
            .queueFamilyIndexCount = static_cast<uint32_t>(exclusiveSharingMode ? 0 : 2),
            .pQueueFamilyIndices = exclusiveSharingMode
                                   ? nullptr
                                   : queueFamilyIndices,
            .preTransform = capabilities.currentTransform,
            .compositeAlpha = VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = presentMode,
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE
    };

    vkEnsure(vkCreateSwapchainKHR(this->device, &swapchainCreateInfo, nullptr, &this->swapchain));
    this->swapchainExtent = extent;

    uint32_t imageCount;
    vkEnsure(vkGetSwapchainImagesKHR(this->device, this->swapchain, &imageCount, nullptr));
    this->swapchainImages.resize(imageCount);
    vkEnsure(vkGetSwapchainImagesKHR(this->device, this->swapchain, &imageCount, this->swapchainImages.data()));

    this->swapchainImageViews.resize(imageCount);
    for (uint32_t idx = 0; idx < imageCount; idx++) {
        this->swapchainImageViews[idx] = createImageView(this->swapchainImages[idx],
                                                         this->_physicalDevice->getColorFormat(),
                                                         VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

void Renderer::initSwapchainResources() {
    // color
    this->colorImage = createImage(this->swapchainExtent.width, this->swapchainExtent.height,
                                   this->_physicalDevice->getColorFormat(),
                                   VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                                   VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                   this->_physicalDevice->getMsaaSamples());
    this->colorImageMemory = allocateMemoryForImage(this->colorImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    this->colorImageView = createImageView(this->colorImage,
                                           this->_physicalDevice->getColorFormat(),
                                           VK_IMAGE_ASPECT_COLOR_BIT);

    // depth
    this->depthImage = createImage(this->swapchainExtent.width, this->swapchainExtent.height,
                                   this->_physicalDevice->getDepthFormat(),
                                   VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                   this->_physicalDevice->getMsaaSamples());
    this->depthImageMemory = allocateMemoryForImage(this->depthImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    this->depthImageView = createImageView(this->depthImage,
                                           this->_physicalDevice->getDepthFormat(),
                                           VK_IMAGE_ASPECT_DEPTH_BIT);
}

void Renderer::initSync() {
    VkFenceCreateInfo fenceCreateInfo = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = VkFenceCreateFlagBits::VK_FENCE_CREATE_SIGNALED_BIT
    };

    VkSemaphoreCreateInfo semaphoreCreateInfo = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0
    };

    for (int idx = 0; idx < VK_MAX_INFLIGHT_FRAMES; idx++) {
        vkEnsure(vkCreateFence(this->device, &fenceCreateInfo, nullptr, &this->fences[idx]));
        vkEnsure(vkCreateSemaphore(this->device, &semaphoreCreateInfo, nullptr, &this->imageAvailableSemaphores[idx]));
        vkEnsure(vkCreateSemaphore(this->device, &semaphoreCreateInfo, nullptr, &this->renderFinishedSemaphores[idx]));
    }
}

void Renderer::initUniformBuffers() {
    VkDeviceSize uboSize = sizeof(UniformBufferObject);

    for (size_t idx = 0; idx < VK_MAX_INFLIGHT_FRAMES; idx++) {
        VkBuffer buffer = createBuffer(this->device, uboSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        VkDeviceMemory memory = allocateMemoryForBuffer(this->device, this->_physicalDevice->getHandle(),
                                                        buffer,
                                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        this->uniformBuffers[idx] = buffer;
        this->uniformBufferMemory[idx] = memory;
        vkMapMemory(this->device, memory, 0, uboSize, 0, &this->uniformBufferMemoryMapped[idx]);
    }
}

void Renderer::initTextureSampler() {
    VkSamplerCreateInfo samplerCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .mipmapMode = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR,
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

    vkEnsure(vkCreateSampler(this->device, &samplerCreateInfo, nullptr, &this->textureSampler));
}

void Renderer::initDescriptors() {
    std::array<VkDescriptorPoolSize, 2> descriptorPoolSizes = {
            VkDescriptorPoolSize{
                    .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .descriptorCount = VK_MAX_INFLIGHT_FRAMES
            },
            VkDescriptorPoolSize{
                    .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .descriptorCount = VK_MAX_INFLIGHT_FRAMES
            }
    };

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
            .maxSets = 4 * VK_MAX_INFLIGHT_FRAMES,
            .poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size()),
            .pPoolSizes = descriptorPoolSizes.data()
    };

    vkEnsure(vkCreateDescriptorPool(this->device, &descriptorPoolCreateInfo, nullptr, &this->descriptorPool));
}

void Renderer::initLayouts() {
    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {
            VkDescriptorSetLayoutBinding{
                    .binding = 0,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                    .pImmutableSamplers = nullptr
            },
            VkDescriptorSetLayoutBinding{
                    .binding = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .pImmutableSamplers = nullptr
            }
    };

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .bindingCount = static_cast<uint32_t>(bindings.size()),
            .pBindings = bindings.data()
    };

    vkEnsure(vkCreateDescriptorSetLayout(this->device, &descriptorSetLayoutCreateInfo, nullptr,
                                         &this->descriptorSetLayout));

    VkPushConstantRange constantRange = {
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .offset = 0,
            .size = sizeof(Constants)
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .setLayoutCount = 1,
            .pSetLayouts = &this->descriptorSetLayout,
            .pushConstantRangeCount = 1,
            .pPushConstantRanges = &constantRange
    };

    vkEnsure(vkCreatePipelineLayout(this->device, &pipelineLayoutInfo, nullptr, &this->pipelineLayout));
}

void Renderer::cleanupSwapchain() {
    for (RenderpassBase *renderpass: this->_renderpasses) {
        renderpass->destroyFramebuffers();
    }

    vkDestroyImageView(this->device, this->colorImageView, nullptr);
    vkFreeMemory(this->device, this->colorImageMemory, nullptr);
    vkDestroyImage(this->device, this->colorImage, nullptr);

    vkDestroyImageView(this->device, this->depthImageView, nullptr);
    vkFreeMemory(this->device, this->depthImageMemory, nullptr);
    vkDestroyImage(this->device, this->depthImage, nullptr);

    for (const VkImageView &imageView: this->swapchainImageViews) {
        vkDestroyImageView(this->device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(this->device, this->swapchain, nullptr);
}

void Renderer::handleResize() {
    vkDeviceWaitIdle(this->device);

    cleanupSwapchain();

    initSwapchain();
    initSwapchainResources();

    uint32_t swapchainImagesCount = this->swapchainImageViews.size();
    Swapchain swapchain_ = {
            .width = this->swapchainExtent.width,
            .height = this->swapchainExtent.height,
            .swapchainImagesCount = swapchainImagesCount
    };

    RenderTargets renderTargets = {
            .colorGroup = std::vector<VkImageView>(swapchainImagesCount, this->colorImageView),
            .depthGroup = std::vector<VkImageView>(swapchainImagesCount, this->depthImageView),
            .resolveGroup = this->swapchainImageViews
    };

    for (RenderpassBase *renderpass: this->_renderpasses) {
        renderpass->createFramebuffers(swapchain_, renderTargets);
    }
}

void Renderer::render() {
    VkFence currentFence = this->fences[frameIdx];
    VkSemaphore currentImageAvailableSemaphore = this->imageAvailableSemaphores[frameIdx];
    VkSemaphore currentRenderFinishedSemaphore = this->renderFinishedSemaphores[frameIdx];

    vkWaitForFences(this->device, 1, &currentFence, VK_TRUE, UINT64_MAX);
    vkResetFences(this->device, 1, &currentFence);

    uint32_t imageIdx;
    VkResult result = vkAcquireNextImageKHR(this->device, this->swapchain, UINT64_MAX,
                                            currentImageAvailableSemaphore,
                                            VK_NULL_HANDLE, &imageIdx);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        handleResize();
        return;
    } else if (result != VkResult::VK_SUCCESS && result != VkResult::VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Vulkan acquire next image failure");
    }

    ubo.view = this->engine->camera().view();
    ubo.proj = glm::perspective(glm::radians(45.0f), this->swapchainExtent.width / (float) this->swapchainExtent.height,
                                0.1f, 10.0f);
    ubo.proj[1][1] *= -1;
    memcpy(this->uniformBufferMemoryMapped[frameIdx], &ubo, sizeof(ubo));

    this->_vulkanCommandExecutor->beginMainExecution(frameIdx, [this, &imageIdx](VkCommandBuffer cmdBuffer) {
                VkRect2D renderArea = VkRect2D{
                        .offset = {0, 0},
                        .extent = this->swapchainExtent
                };

                for (RenderpassBase *renderpass: this->_renderpasses) {
                    renderpass->recordCommands(cmdBuffer, renderArea, frameIdx, imageIdx);
                }
            })
            .withFence(currentFence)
            .withWaitSemaphore(currentImageAvailableSemaphore)
            .withSignalSemaphore(currentRenderFinishedSemaphore)
            .withWaitDstStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
            .submit(false);

    VkPresentInfoKHR presentInfo = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &currentRenderFinishedSemaphore,
            .swapchainCount = 1,
            .pSwapchains = &this->swapchain,
            .pImageIndices = &imageIdx,
            .pResults = nullptr
    };

    result = vkQueuePresentKHR(this->presentQueue, &presentInfo);

    if (result == VkResult::VK_ERROR_OUT_OF_DATE_KHR || result == VkResult::VK_SUBOPTIMAL_KHR || resizeRequested) {
        resizeRequested = false;
        handleResize();
    } else if (result != VkResult::VK_SUCCESS) {
        throw std::runtime_error("Vulkan present failure");
    }

    frameIdx = (frameIdx + 1) % VK_MAX_INFLIGHT_FRAMES;
}

BufferData Renderer::uploadVertices(const std::vector<Vertex> &vertices) {
    // TODO: use staging buffer to restrict usage of GPU memory by CPU

    VkDeviceSize size = sizeof(vertices[0]) * vertices.size();

    VkBuffer buffer = createBuffer(this->device, size, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                                       VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VkDeviceMemory memory = allocateMemoryForBuffer(this->device, this->_physicalDevice->getHandle(),
                                                    buffer,
                                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    void *data;
    vkMapMemory(this->device, memory, 0, size, 0, &data);
    memcpy(data, vertices.data(), size);
    vkUnmapMemory(this->device, memory);

    return {
            .buffer = buffer,
            .memory = memory
    };
}

BufferData Renderer::uploadIndices(const std::vector<uint32_t> &indices) {
    // TODO: use staging buffer to restrict usage of GPU memory by CPU

    VkDeviceSize size = sizeof(indices[0]) * indices.size();

    VkBuffer buffer = createBuffer(this->device, size, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                                       VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    VkDeviceMemory memory = allocateMemoryForBuffer(this->device, this->_physicalDevice->getHandle(),
                                                    buffer,
                                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    void *data;
    vkMapMemory(this->device, memory, 0, size, 0, &data);
    memcpy(data, indices.data(), size);
    vkUnmapMemory(this->device, memory);

    return {
            .buffer = buffer,
            .memory = memory
    };
}

TextureData Renderer::uploadTexture(const std::string &texturePath) {
    int width, height, channels;
    stbi_uc *pixels = stbi_load(texturePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    VkDeviceSize imageSize = width * height * 4;

    VkBuffer stagingBuffer = createBuffer(this->device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    VkDeviceMemory stagingBufferMemory = allocateMemoryForBuffer(this->device, this->_physicalDevice->getHandle(),
                                                                 stagingBuffer,
                                                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *data;
    vkMapMemory(this->device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(this->device, stagingBufferMemory);
    stbi_image_free(pixels);

    VkImage image = createImage(width, height, VK_FORMAT_R8G8B8A8_SRGB,
                                VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                VK_IMAGE_USAGE_SAMPLED_BIT,
                                VK_SAMPLE_COUNT_1_BIT);
    VkDeviceMemory imageMemory = allocateMemoryForImage(image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    this->_vulkanCommandExecutor->beginOneTimeExecution(
                    [&image, &width, &height, &stagingBuffer](VkCommandBuffer cmdBuffer) {
                        VkImageMemoryBarrier imageMemoryBarrier = {
                                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                                .pNext = nullptr,
                                .srcAccessMask = 0,
                                .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                .image = image,
                                .subresourceRange = {
                                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                        .baseMipLevel = 0,
                                        .levelCount = 1,
                                        .baseArrayLayer = 0,
                                        .layerCount = 1
                                }
                        };
                        vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                                             0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

                        VkBufferImageCopy bufferImageCopy = {
                                .bufferOffset = 0,
                                .bufferRowLength = 0,
                                .bufferImageHeight = 0,
                                .imageSubresource = {
                                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                        .mipLevel = 0,
                                        .baseArrayLayer = 0,
                                        .layerCount = 1
                                },
                                .imageOffset = {0, 0, 0},
                                .imageExtent = {(uint32_t) width, (uint32_t) height, 1}
                        };
                        vkCmdCopyBufferToImage(cmdBuffer, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                                               &bufferImageCopy);

                        VkImageMemoryBarrier anotherImageMemoryBarrier = {
                                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                                .pNext = nullptr,
                                .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                                .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
                                .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                .image = image,
                                .subresourceRange = {
                                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                        .baseMipLevel = 0,
                                        .levelCount = 1,
                                        .baseArrayLayer = 0,
                                        .layerCount = 1
                                }
                        };
                        vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                             0, 0, nullptr, 0, nullptr, 1, &anotherImageMemoryBarrier);

                    })
            .submit(true);

    vkFreeMemory(this->device, stagingBufferMemory, nullptr);
    vkDestroyBuffer(this->device, stagingBuffer, nullptr);

    VkImageView imageView = createImageView(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

    return TextureData{
            .image = image,
            .imageView = imageView,
            .memory = imageMemory
    };
}

std::array<VkDescriptorSet, VK_MAX_INFLIGHT_FRAMES> Renderer::initDescriptorSets(VkImageView textureImageView) {
    std::vector<VkDescriptorSetLayout> layouts(VK_MAX_INFLIGHT_FRAMES, this->descriptorSetLayout);
    std::array<VkDescriptorSet, VK_MAX_INFLIGHT_FRAMES> descriptorSets = {};

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = this->descriptorPool,
            .descriptorSetCount = static_cast<uint32_t>(descriptorSets.size()),
            .pSetLayouts = layouts.data()
    };

    vkEnsure(vkAllocateDescriptorSets(this->device, &descriptorSetAllocateInfo, descriptorSets.data()));

    for (uint32_t idx = 0; idx < VK_MAX_INFLIGHT_FRAMES; idx++) {
        VkDescriptorBufferInfo bufferInfo = {
                .buffer = this->uniformBuffers[idx],
                .offset = 0,
                .range = sizeof(UniformBufferObject)
        };

        VkDescriptorImageInfo imageInfo = {
                .sampler = this->textureSampler,
                .imageView = textureImageView,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };

        std::array<VkWriteDescriptorSet, 2> writes = {
                VkWriteDescriptorSet{
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .pNext = nullptr,
                        .dstSet = descriptorSets[idx],
                        .dstBinding = 0,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        .pImageInfo = nullptr,
                        .pBufferInfo = &bufferInfo,
                        .pTexelBufferView = nullptr
                },
                VkWriteDescriptorSet{
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .pNext = nullptr,
                        .dstSet = descriptorSets[idx],
                        .dstBinding = 1,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        .pImageInfo = &imageInfo,
                        .pBufferInfo = nullptr,
                        .pTexelBufferView = nullptr
                }
        };

        vkUpdateDescriptorSets(this->device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
    }

    return descriptorSets;
}

BoundMeshInfo *Renderer::uploadMesh(const Mesh &mesh, const Texture &texture) {
    TextureData textureData = uploadTexture(texture.path);

    auto boundMeshInfo = new BoundMeshInfo{
            .vertexBuffer = uploadVertices(mesh.vertices),
            .indexBuffer = uploadIndices(mesh.indices),
            .texture = textureData,
            .model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f)),
            .indicesCount = static_cast<uint32_t>(mesh.indices.size()),
            .descriptorSets = initDescriptorSets(textureData.imageView)
    };

    this->_sceneRenderpass->addMesh(boundMeshInfo);

    return boundMeshInfo;
}

void Renderer::freeMesh(BoundMeshInfo *meshInfo) {
    vkEnsure(vkDeviceWaitIdle(this->device));

    vkFreeDescriptorSets(this->device, this->descriptorPool, static_cast<uint32_t>(meshInfo->descriptorSets.size()),
                         meshInfo->descriptorSets.data());

    vkFreeMemory(this->device, meshInfo->texture.memory, nullptr);
    vkDestroyImageView(this->device, meshInfo->texture.imageView, nullptr);
    vkDestroyImage(this->device, meshInfo->texture.image, nullptr);

    vkFreeMemory(this->device, meshInfo->vertexBuffer.memory, nullptr);
    vkDestroyBuffer(this->device, meshInfo->vertexBuffer.buffer, nullptr);

    vkFreeMemory(this->device, meshInfo->indexBuffer.memory, nullptr);
    vkDestroyBuffer(this->device, meshInfo->indexBuffer.buffer, nullptr);

    this->_sceneRenderpass->removeMesh(meshInfo);
}
