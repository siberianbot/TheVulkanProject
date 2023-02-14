#include "Renderer.hpp"

#include <set>
#include <iostream>
#include <array>
#include <limits>
#include <fstream>

#include "Constants.hpp"
#include "Engine.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

static constexpr VkFormat VK_DEPTH_FORMATS[] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
};

static constexpr const char *VK_DEFAULT_VERTEX_SHADER = "shaders/default.vert.spv";
static constexpr const char *VK_DEFAULT_FRAGMENT_SHADER = "shaders/default.frag.spv";

static constexpr void vkEnsure(VkResult vkExpression) {
    if (vkExpression != VkResult::VK_SUCCESS) {
        throw std::runtime_error("Vulkan runtime error");
    }
}

VkSurfaceFormatKHR getPreferredSurfaceFormat(std::vector<VkSurfaceFormatKHR> formats) {
    auto it = std::find_if(formats.begin(), formats.end(), [](const VkSurfaceFormatKHR &format) {
        return format.colorSpace == VkColorSpaceKHR::VK_COLOR_SPACE_SRGB_NONLINEAR_KHR &&
               format.format == VkFormat::VK_FORMAT_B8G8R8A8_SRGB;
    });

    return it != formats.end()
           ? *it
           : formats[0];
}

VkPresentModeKHR getPreferredPresentMode(std::vector<VkPresentModeKHR> presentModes) {
    auto it = std::find(presentModes.begin(), presentModes.end(), VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR);

    return it != presentModes.end()
           ? *it
           : VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;
}

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

uint32_t Renderer::getSuitableMemoryType(uint32_t memoryType, VkMemoryPropertyFlags memoryProperty) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(this->physicalDevice, &memoryProperties);

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

VkSampleCountFlagBits getSuitableSampleCount(VkPhysicalDeviceProperties deviceProperties) {
    VkSampleCountFlags counts = deviceProperties.limits.framebufferColorSampleCounts &
                                deviceProperties.limits.framebufferDepthSampleCounts;

    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

    return VK_SAMPLE_COUNT_1_BIT;
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

PhysicalDeviceInfo Renderer::getPhysicalDeviceInfo(VkPhysicalDevice device) {
    PhysicalDeviceInfo info = {};
    uint32_t count;

    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
    std::vector<VkQueueFamilyProperties> familyProperties(count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, familyProperties.data());

    for (uint32_t idx = 0; idx < count; idx++) {
        VkQueueFamilyProperties props = familyProperties[idx];

        if (props.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT) {
            info.graphicsFamilyIdx = idx;
        }

        VkBool32 isPresentSupported = false;
        vkEnsure(vkGetPhysicalDeviceSurfaceSupportKHR(device, idx, this->surface, &isPresentSupported));

        if (isPresentSupported) {
            info.presentFamilyIdx = idx;
        }

        if (info.graphicsFamilyIdx.has_value() && info.presentFamilyIdx.has_value()) {
            break;
        }
    }

    vkEnsure(vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr));
    std::vector<VkExtensionProperties> extensions(count);
    vkEnsure(vkEnumerateDeviceExtensionProperties(device, nullptr, &count, extensions.data()));

    for (VkExtensionProperties extension: extensions) {
        info.extensions.emplace_back(extension.extensionName);
    }

    vkEnsure(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, this->surface, &info.capabilities));

    vkEnsure(vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->surface, &count, nullptr));
    if (count > 0) {
        info.surfaceFormats.resize(count);
        vkEnsure(vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->surface, &count, info.surfaceFormats.data()));
    }

    vkEnsure(vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->surface, &count, nullptr));
    if (count > 0) {
        info.presentModes.resize(count);
        vkEnsure(vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->surface, &count, info.presentModes.data()));
    }

    for (VkFormat depthFormat: VK_DEPTH_FORMATS) {
        VkFormatProperties formatProps;
        vkGetPhysicalDeviceFormatProperties(device, depthFormat, &formatProps);

        if (formatProps.optimalTilingFeatures &
            VkFormatFeatureFlagBits::VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            info.depthFormat = depthFormat;
        }

        if (info.depthFormat.has_value()) {
            break;
        }
    }

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    info.msaaSamples = getSuitableSampleCount(deviceProperties);
    info.maxSamplerAnisotropy = deviceProperties.limits.maxSamplerAnisotropy;

    return info;
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

VkShaderModule Renderer::createShaderModule(const std::string &path) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("shader file read failure");
    }

    size_t size = file.tellg();
    std::vector<char> buffer(size);

    file.seekg(0);
    file.read(buffer.data(), size);
    file.close();

    VkShaderModuleCreateInfo shaderModuleCreateInfo = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .codeSize = buffer.size(),
            .pCode = reinterpret_cast<const uint32_t *>(buffer.data())
    };

    VkShaderModule shaderModule;
    vkEnsure(vkCreateShaderModule(this->device, &shaderModuleCreateInfo, nullptr, &shaderModule));

    return shaderModule;
}

VkBuffer Renderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage) {
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
    vkEnsure(vkCreateBuffer(this->device, &bufferCreateInfo, nullptr, &buffer));

    return buffer;
}

VkDeviceMemory Renderer::allocateMemoryForImage(VkImage image, VkMemoryPropertyFlags memoryProperty) {
    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(this->device, image, &memoryRequirements);

    uint32_t memoryType = getSuitableMemoryType(memoryRequirements.memoryTypeBits, memoryProperty);

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

VkDeviceMemory Renderer::allocateMemoryForBuffer(VkBuffer buffer, VkMemoryPropertyFlags memoryProperty) {
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(this->device, buffer, &memoryRequirements);

    uint32_t memoryType = getSuitableMemoryType(memoryRequirements.memoryTypeBits, memoryProperty);

    VkMemoryAllocateInfo memoryAllocateInfo = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = nullptr,
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = memoryType
    };

    VkDeviceMemory memory;
    vkEnsure(vkAllocateMemory(this->device, &memoryAllocateInfo, nullptr, &memory));
    vkEnsure(vkBindBufferMemory(this->device, buffer, memory, 0));

    return memory;
}

Renderer::Renderer(Engine *engine) : engine(engine) {
    //
}

void Renderer::init() {
    initInstance();

    initSurface(this->engine->window());

    initPhysicalDevice();
    initDevice();

    initCommand();
    initSync();

    initUniformBuffers();
    initTextureSampler();
    initDescriptors();
    initLayouts();

    initSwapchain();
    initSwapchainResources();

    for (auto & renderpass : this->renderpasses) {
        renderpass = createRenderpass();
        createRenderpassFramebuffers(renderpass);
    }
}

void Renderer::cleanup() {
    vkEnsure(vkDeviceWaitIdle(this->device));

    cleanupSwapchain();

    for (const auto &renderpass: this->renderpasses) {
        destroyRenderpass(renderpass);
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

    vkFreeCommandBuffers(this->device, this->commandPool, VK_MAX_INFLIGHT_FRAMES, this->commandBuffers.data());
    vkDestroyCommandPool(this->device, this->commandPool, nullptr);
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

void Renderer::initPhysicalDevice() {
    uint32_t count;
    vkEnsure(vkEnumeratePhysicalDevices(this->instance, &count, nullptr));

    if (count == 0) {
        throw std::runtime_error("no physical device available");
    }

    std::vector<VkPhysicalDevice> devices(count);
    vkEnsure(vkEnumeratePhysicalDevices(this->instance, &count, devices.data()));

    for (VkPhysicalDevice device: devices) {
        PhysicalDeviceInfo info = getPhysicalDeviceInfo(device);

        if (!info.isSuitable()) {
            continue;
        }

        this->physicalDevice = device;
        this->physicalDeviceInfo = info;
    }

    if (this->physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("no physical device available");
    }
}

void Renderer::initDevice() {
    const float queuePriority = 1.0f;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> familyIndices = {
            this->physicalDeviceInfo.graphicsFamilyIdx.value(),
            this->physicalDeviceInfo.presentFamilyIdx.value()
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

    vkEnsure(vkCreateDevice(this->physicalDevice, &deviceCreateInfo, nullptr, &this->device));
    vkGetDeviceQueue(this->device, this->physicalDeviceInfo.graphicsFamilyIdx.value(), 0, &this->graphicsQueue);
    vkGetDeviceQueue(this->device, this->physicalDeviceInfo.presentFamilyIdx.value(), 0, &this->presentQueue);
}

void Renderer::initSwapchain() {
    VkSurfaceFormatKHR surfaceFormat = getPreferredSurfaceFormat(this->physicalDeviceInfo.surfaceFormats);
    VkPresentModeKHR presentMode = getPreferredPresentMode(this->physicalDeviceInfo.presentModes);
    VkExtent2D extent = getPreferredExtent(this->physicalDeviceInfo.capabilities, this->currentExtent);

    uint32_t minImageCount = this->physicalDeviceInfo.capabilities.minImageCount + 1;
    if (this->physicalDeviceInfo.capabilities.maxImageCount > 0 &&
        minImageCount > this->physicalDeviceInfo.capabilities.maxImageCount) {
        minImageCount = this->physicalDeviceInfo.capabilities.maxImageCount;
    }

    bool exclusiveSharingMode = this->physicalDeviceInfo.presentFamilyIdx.value() ==
                                this->physicalDeviceInfo.graphicsFamilyIdx.value();

    uint32_t queueFamilyIndices[] = {
            this->physicalDeviceInfo.presentFamilyIdx.value(),
            this->physicalDeviceInfo.graphicsFamilyIdx.value()
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
            .preTransform = this->physicalDeviceInfo.capabilities.currentTransform,
            .compositeAlpha = VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = presentMode,
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE
    };

    vkEnsure(vkCreateSwapchainKHR(this->device, &swapchainCreateInfo, nullptr, &this->swapchain));
    this->swapchainFormat = surfaceFormat.format;
    this->swapchainExtent = extent;

    uint32_t imageCount;
    vkEnsure(vkGetSwapchainImagesKHR(this->device, this->swapchain, &imageCount, nullptr));
    this->swapchainImages.resize(imageCount);
    vkEnsure(vkGetSwapchainImagesKHR(this->device, this->swapchain, &imageCount, this->swapchainImages.data()));

    this->swapchainImageViews.resize(imageCount);
    for (uint32_t idx = 0; idx < imageCount; idx++) {
        this->swapchainImageViews[idx] = createImageView(this->swapchainImages[idx],
                                                         this->swapchainFormat,
                                                         VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

VkRenderPass Renderer::initRenderPass() {
    VkAttachmentDescription colorAttachment = {
            .flags = 0,
            .format= this->swapchainFormat,
            .samples = this->physicalDeviceInfo.msaaSamples,
            .loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkAttachmentReference colorAttachmentReference = {
            .attachment = 0,
            .layout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkAttachmentDescription depthAttachment = {
            .flags = 0,
            .format= this->physicalDeviceInfo.depthFormat.value(),
            .samples = this->physicalDeviceInfo.msaaSamples,
            .loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    VkAttachmentReference depthAttachmentReference = {
            .attachment = 1,
            .layout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    VkAttachmentDescription colorAttachmentResolve = {
            .flags = 0,
            .format= this->swapchainFormat,
            .samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference colorAttachmentResolveReference = {
            .attachment = 2,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass = {
            .flags = 0,
            .pipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = 0,
            .pInputAttachments = nullptr,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentReference,
            .pResolveAttachments = &colorAttachmentResolveReference,
            .pDepthStencilAttachment = &depthAttachmentReference,
            .preserveAttachmentCount = 0,
            .pPreserveAttachments = nullptr
    };

    VkSubpassDependency subpassDependency = {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .srcAccessMask = VkAccessFlagBits::VK_ACCESS_NONE,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dependencyFlags = 0
    };

    std::array<VkAttachmentDescription, 3> attachments = {
            colorAttachment,
            depthAttachment,
            colorAttachmentResolve
    };

    VkRenderPassCreateInfo renderPassCreateInfo = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .attachmentCount = attachments.size(),
            .pAttachments = attachments.data(),
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = 1,
            .pDependencies = &subpassDependency
    };

    VkRenderPass renderpass;
    vkEnsure(vkCreateRenderPass(this->device, &renderPassCreateInfo, nullptr, &renderpass));

    return renderpass;
}

void Renderer::initCommand() {
    VkCommandPoolCreateInfo commandPoolCreateInfo = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = this->physicalDeviceInfo.graphicsFamilyIdx.value()
    };

    vkEnsure(vkCreateCommandPool(this->device, &commandPoolCreateInfo, nullptr, &this->commandPool));

    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = this->commandPool,
            .level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = VK_MAX_INFLIGHT_FRAMES
    };

    vkEnsure(vkAllocateCommandBuffers(this->device, &commandBufferAllocateInfo, this->commandBuffers.data()));
}

void Renderer::initSwapchainResources() {
    // color
    this->colorImage = createImage(this->swapchainExtent.width, this->swapchainExtent.height,
                                   this->swapchainFormat,
                                   VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                                   VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                   this->physicalDeviceInfo.msaaSamples);
    this->colorImageMemory = allocateMemoryForImage(this->colorImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    this->colorImageView = createImageView(this->colorImage, this->swapchainFormat,
                                           VK_IMAGE_ASPECT_COLOR_BIT);

    // depth
    this->depthImage = createImage(this->swapchainExtent.width, this->swapchainExtent.height,
                                   this->physicalDeviceInfo.depthFormat.value(),
                                   VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                   this->physicalDeviceInfo.msaaSamples);
    this->depthImageMemory = allocateMemoryForImage(this->depthImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    this->depthImageView = createImageView(this->depthImage, this->physicalDeviceInfo.depthFormat.value(),
                                           VK_IMAGE_ASPECT_DEPTH_BIT);
}

void Renderer::initFramebuffers(Renderpass &renderpass) {
    uint32_t count = this->swapchainImageViews.size();

    renderpass.framebuffers.resize(count);

    for (uint32_t idx = 0; idx < count; idx++) {
        std::array<VkImageView, 3> attachments = {
                this->colorImageView,
                this->depthImageView,
                this->swapchainImageViews[idx]
        };

        VkFramebufferCreateInfo framebufferCreateInfo = {
                .sType = VkStructureType::VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .renderPass = renderpass.renderpass,
                .attachmentCount = 3,
                .pAttachments = attachments.data(),
                .width = this->swapchainExtent.width,
                .height = this->swapchainExtent.height,
                .layers = 1
        };

        vkEnsure(vkCreateFramebuffer(this->device, &framebufferCreateInfo, nullptr, &renderpass.framebuffers[idx]));
    }
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

std::array<VkClearValue, 2> Renderer::initClearColors() {
    std::array<VkClearValue, 2> clearColors = {};

    clearColors[0].color = {{0.0f, 0.0f, 0.0f, 0.0f}};
    clearColors[1].depthStencil = {1.0f, 0};

    return clearColors;
}

void Renderer::initUniformBuffers() {
    VkDeviceSize uboSize = sizeof(UniformBufferObject);

    for (size_t idx = 0; idx < VK_MAX_INFLIGHT_FRAMES; idx++) {
        VkBuffer buffer = createBuffer(uboSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        VkDeviceMemory memory = allocateMemoryForBuffer(buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
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
            .maxAnisotropy = this->physicalDeviceInfo.maxSamplerAnisotropy,
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

VkPipeline Renderer::initGraphicsPipeline(VkRenderPass renderpass) {
    VkShaderModule vertexShaderModule = createShaderModule(VK_DEFAULT_VERTEX_SHADER);
    VkShaderModule fragmentShaderModule = createShaderModule(VK_DEFAULT_FRAGMENT_SHADER);

    VkPipelineShaderStageCreateInfo vertexShaderStage = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertexShaderModule,
            .pName = "main",
            .pSpecializationInfo = nullptr
    };

    VkPipelineShaderStageCreateInfo fragmentShaderStage = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragmentShaderModule,
            .pName = "main",
            .pSpecializationInfo = nullptr
    };

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {
            vertexShaderStage,
            fragmentShaderStage
    };

    VkVertexInputBindingDescription bindingDescription = {
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX
    };

    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {
            VkVertexInputAttributeDescription{
                    .location = 0,
                    .binding = 0,
                    .format = VK_FORMAT_R32G32B32_SFLOAT,
                    .offset = offsetof(Vertex, pos)
            },
            VkVertexInputAttributeDescription{
                    .location = 1,
                    .binding = 0,
                    .format = VK_FORMAT_R32G32B32_SFLOAT,
                    .offset = offsetof(Vertex, color)
            },
            VkVertexInputAttributeDescription{
                    .location = 2,
                    .binding = 0,
                    .format = VK_FORMAT_R32G32_SFLOAT,
                    .offset = offsetof(Vertex, texCoord)
            }
    };

    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &bindingDescription,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
            .pVertexAttributeDescriptions = attributeDescriptions.data()
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE
    };

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .viewportCount = 1,
            .pViewports = nullptr,
            .scissorCount = 1,
            .pScissors = nullptr
    };

    VkPipelineRasterizationStateCreateInfo rasterizer = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = 0,
            .depthBiasClamp = 0,
            .depthBiasSlopeFactor = 0,
            .lineWidth = 1.0f,
    };

    VkPipelineMultisampleStateCreateInfo multisampling = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .rasterizationSamples = this->physicalDeviceInfo.msaaSamples,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 0,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE
    };

    VkPipelineDepthStencilStateCreateInfo depthStencil = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .depthTestEnable = VK_TRUE,
            .depthWriteEnable = VK_TRUE,
            .depthCompareOp = VK_COMPARE_OP_LESS,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE,
            .front = {},
            .back = {},
            .minDepthBounds = 0,
            .maxDepthBounds = 0
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
            .blendEnable = VK_FALSE,
            .srcColorBlendFactor = {},
            .dstColorBlendFactor = {},
            .colorBlendOp = VkBlendOp::VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = {},
            .dstAlphaBlendFactor = {},
            .alphaBlendOp = VkBlendOp::VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                              VK_COLOR_COMPONENT_G_BIT |
                              VK_COLOR_COMPONENT_B_BIT |
                              VK_COLOR_COMPONENT_A_BIT
    };

    VkPipelineColorBlendStateCreateInfo colorBlending = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = &colorBlendAttachment,
            .blendConstants = {0, 0, 0, 0}
    };

    std::array<VkDynamicState, 2> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
            .pDynamicStates = dynamicStates.data()
    };

    VkGraphicsPipelineCreateInfo pipelineInfo = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stageCount = static_cast<uint32_t>(shaderStages.size()),
            .pStages = shaderStages.data(),
            .pVertexInputState = &vertexInputStateCreateInfo,
            .pInputAssemblyState = &inputAssemblyStateCreateInfo,
            .pTessellationState = nullptr,
            .pViewportState = &viewportStateCreateInfo,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pDepthStencilState = &depthStencil,
            .pColorBlendState = &colorBlending,
            .pDynamicState = &dynamicState,
            .layout = this->pipelineLayout,
            .renderPass = renderpass,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = 0
    };

    VkPipeline pipeline;
    vkEnsure(vkCreateGraphicsPipelines(this->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline));

    vkDestroyShaderModule(this->device, vertexShaderModule, nullptr);
    vkDestroyShaderModule(this->device, fragmentShaderModule, nullptr);

    return pipeline;
}

void Renderer::destroyRenderpass(const Renderpass &renderpass) {
    vkDestroyPipeline(this->device, renderpass.pipeline, nullptr);

    vkDestroyRenderPass(this->device, renderpass.renderpass, nullptr);
}

void Renderer::destroyRenderpassFramebuffers(const Renderpass &renderpass) {
    for (const VkFramebuffer &framebuffer: renderpass.framebuffers) {
        vkDestroyFramebuffer(this->device, framebuffer, nullptr);
    }
}

void Renderer::cleanupSwapchain() {
    for (const Renderpass &renderpass: this->renderpasses) {
        destroyRenderpassFramebuffers(renderpass);
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

    this->physicalDeviceInfo = getPhysicalDeviceInfo(this->physicalDevice);

    initSwapchain();
    initSwapchainResources();

    for (auto & renderpass : this->renderpasses) {
        createRenderpassFramebuffers(renderpass);
    }
}

void Renderer::render() {
    VkFence currentFence = this->fences[currentFrame];
    VkSemaphore currentImageAvailableSemaphore = this->imageAvailableSemaphores[currentFrame];
    VkSemaphore currentRenderFinishedSemaphore = this->renderFinishedSemaphores[currentFrame];
    VkCommandBuffer currentCommandBuffer = this->commandBuffers[currentFrame];

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
    memcpy(this->uniformBufferMemoryMapped[currentFrame], &ubo, sizeof(ubo));

    vkResetCommandBuffer(currentCommandBuffer, 0);

    VkCommandBufferBeginInfo commandBufferBeginInfo = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pInheritanceInfo = nullptr
    };
    vkEnsure(vkBeginCommandBuffer(currentCommandBuffer, &commandBufferBeginInfo));

    for (uint32_t renderpassIdx = 0; renderpassIdx < this->renderpasses.size(); renderpassIdx++) {
        Renderpass renderpass = this->renderpasses[renderpassIdx];

        VkRenderPassBeginInfo renderPassBeginInfo = {
                .sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                .pNext = nullptr,
                .renderPass = renderpass.renderpass,
                .framebuffer = renderpass.framebuffers[imageIdx],
                .renderArea = {
                        .offset = {0, 0},
                        .extent = this->swapchainExtent
                },
                .clearValueCount = static_cast<uint32_t>(renderpass.clearValues.size()),
                .pClearValues = renderpass.clearValues.data()
        };
        vkCmdBeginRenderPass(currentCommandBuffer, &renderPassBeginInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderpass.pipeline);

        VkViewport viewport = {
                .x = 0,
                .y = 0,
                .width = (float) this->swapchainExtent.width,
                .height = (float) this->swapchainExtent.height,
                .minDepth = 0.0f,
                .maxDepth = 1.0f
        };
        vkCmdSetViewport(currentCommandBuffer, 0, 1, &viewport);

        VkRect2D scissor = {
                .offset = {0, 0},
                .extent = this->swapchainExtent
        };
        vkCmdSetScissor(currentCommandBuffer, 0, 1, &scissor);

        int idx = 0;
        for (const auto &mesh: this->meshes) {
//            if (mesh->renderpassIdx != renderpassIdx) {
//                continue;
//            }

            VkDeviceSize offset = 0;
            VkDescriptorSet descriptorSet = mesh->descriptorSets[currentFrame];

            Constants constants = {
                    .model = mesh->model
            };
            vkCmdPushConstants(currentCommandBuffer, pipelineLayout,
                               VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Constants), &constants);

            vkCmdBindVertexBuffers(currentCommandBuffer, 0, 1, &mesh->vertexBuffer.buffer, &offset);
            vkCmdBindIndexBuffer(currentCommandBuffer, mesh->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

            vkCmdBindDescriptorSets(currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                                    0, 1, &descriptorSet, 0, nullptr);

            vkCmdDrawIndexed(currentCommandBuffer, mesh->indicesCount, 1, 0, 0, idx++);
        }

        vkCmdEndRenderPass(currentCommandBuffer);
    }

    vkEnsure(vkEndCommandBuffer(currentCommandBuffer));

    VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &currentImageAvailableSemaphore,
            .pWaitDstStageMask = &waitDstStageMask,
            .commandBufferCount = 1,
            .pCommandBuffers = &currentCommandBuffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &currentRenderFinishedSemaphore
    };
    vkEnsure(vkQueueSubmit(this->graphicsQueue, 1, &submitInfo, currentFence));

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

    currentFrame = (currentFrame + 1) % VK_MAX_INFLIGHT_FRAMES;
}

BufferData Renderer::uploadVertices(const std::vector<Vertex> &vertices) {
    // TODO: use staging buffer to restrict usage of GPU memory by CPU

    VkDeviceSize size = sizeof(vertices[0]) * vertices.size();

    VkBuffer buffer = createBuffer(size, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                         VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VkDeviceMemory memory = allocateMemoryForBuffer(buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
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

    VkBuffer buffer = createBuffer(size, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                         VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    VkDeviceMemory memory = allocateMemoryForBuffer(buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
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

    VkBuffer stagingBuffer = createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    VkDeviceMemory stagingBufferMemory = allocateMemoryForBuffer(stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
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

    VkCommandBuffer tempCommandBuffer;
    VkCommandBufferAllocateInfo tempCommandBufferAllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = this->commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
    };
    vkEnsure(vkAllocateCommandBuffers(this->device, &tempCommandBufferAllocateInfo, &tempCommandBuffer));

    VkCommandBufferBeginInfo tempCommandBufferBeginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            .pInheritanceInfo = nullptr
    };
    vkEnsure(vkBeginCommandBuffer(tempCommandBuffer, &tempCommandBufferBeginInfo));

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
    vkCmdPipelineBarrier(tempCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
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
    vkCmdCopyBufferToImage(tempCommandBuffer, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
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
    vkCmdPipelineBarrier(tempCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &anotherImageMemoryBarrier);

    vkEnsure(vkEndCommandBuffer(tempCommandBuffer));

    VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = nullptr,
            .pWaitDstStageMask = nullptr,
            .commandBufferCount = 1,
            .pCommandBuffers = &tempCommandBuffer,
            .signalSemaphoreCount = 0,
            .pSignalSemaphores = nullptr
    };
    vkEnsure(vkQueueSubmit(this->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
    vkEnsure(vkQueueWaitIdle(this->graphicsQueue));

    vkFreeCommandBuffers(this->device, this->commandPool, 1, &tempCommandBuffer);
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

BoundMeshInfo *Renderer::uploadMesh(uint32_t renderpassIdx, const Mesh &mesh, const Texture &texture) {
    TextureData textureData = uploadTexture(texture.path);

    auto boundMeshInfo = new BoundMeshInfo{
            .renderpassIdx = renderpassIdx,
            .vertexBuffer = uploadVertices(mesh.vertices),
            .indexBuffer = uploadIndices(mesh.indices),
            .texture = textureData,
            .model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f)),
            .indicesCount = static_cast<uint32_t>(mesh.indices.size()),
            .descriptorSets = initDescriptorSets(textureData.imageView)
    };

    this->meshes.push_back(boundMeshInfo);

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

    this->meshes.erase(std::remove(this->meshes.begin(), this->meshes.end(), meshInfo));
}

Renderpass Renderer::createRenderpass() {
    VkRenderPass renderpass = initRenderPass();

    Renderpass result = {
            .renderpass = renderpass,
            .clearValues = initClearColors(),
            .pipeline = initGraphicsPipeline(renderpass)
    };

    return result;
}

void Renderer::createRenderpassFramebuffers(Renderpass &renderpass) {
    initFramebuffers(renderpass);
}
