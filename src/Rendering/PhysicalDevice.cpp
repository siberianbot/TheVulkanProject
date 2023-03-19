#include "PhysicalDevice.hpp"

#include <set>

#include "Common.hpp"
#include "RenderingDevice.hpp"

static constexpr VkFormat PREFERRED_DEPTH_FORMATS[] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
};

PhysicalDevice::PhysicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                               uint32_t graphicsQueueFamilyIdx, uint32_t presentQueueFamilyIdx)
        : _physicalDevice(physicalDevice),
          _surface(surface),
          _graphicsQueueFamilyIdx(graphicsQueueFamilyIdx),
          _presentQueueFamilyIdx(presentQueueFamilyIdx) {
    //
}

PhysicalDevice::QueueFamilies PhysicalDevice::getQueueFamilies(VkPhysicalDevice physicalDevice,
                                                               VkSurfaceKHR surface) {
    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);

    std::vector<VkQueueFamilyProperties> propertiesCollection(count);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, propertiesCollection.data());

    PhysicalDevice::QueueFamilies queueFamilies;
    for (uint32_t idx = 0; idx < count; idx++) {
        VkQueueFamilyProperties properties = propertiesCollection[idx];

        if (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queueFamilies.graphicsIdx = idx;
        }

        VkBool32 isPresentSupported = false;
        vkEnsure(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, idx, surface, &isPresentSupported));

        if (isPresentSupported) {
            queueFamilies.presentIdx = idx;
        }

        if (queueFamilies.isCompleted()) {
            break;
        }
    }

    return queueFamilies;
}

std::vector<std::string> PhysicalDevice::getExtensions(VkPhysicalDevice physicalDevice) {
    uint32_t count;
    vkEnsure(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr));

    std::vector<VkExtensionProperties> propertiesCollection(count);
    vkEnsure(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, propertiesCollection.data()));

    std::vector<std::string> extensions(count);
    for (uint32_t idx = 0; idx < count; idx++) {
        extensions[idx] = propertiesCollection[idx].extensionName;
    }

    return extensions;
}

VkPhysicalDeviceProperties PhysicalDevice::getProperties() {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(this->_physicalDevice, &properties);

    return properties;
}

VkSurfaceCapabilitiesKHR PhysicalDevice::getSurfaceCapabilities() {
    VkSurfaceCapabilitiesKHR capabilities;
    vkEnsure(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(this->_physicalDevice, this->_surface, &capabilities));

    return capabilities;
}

std::vector<VkPresentModeKHR> PhysicalDevice::getPresentModes() {
    uint32_t count;
    vkEnsure(vkGetPhysicalDeviceSurfacePresentModesKHR(this->_physicalDevice, this->_surface, &count, nullptr));

    std::vector<VkPresentModeKHR> presentModes(count);
    vkEnsure(vkGetPhysicalDeviceSurfacePresentModesKHR(this->_physicalDevice, this->_surface, &count,
                                                       presentModes.data()));

    return presentModes;
}

std::vector<VkSurfaceFormatKHR> PhysicalDevice::getSurfaceFormats() {
    uint32_t count;
    vkEnsure(vkGetPhysicalDeviceSurfaceFormatsKHR(this->_physicalDevice, this->_surface, &count, nullptr));

    std::vector<VkSurfaceFormatKHR> surfaceFormats(count);
    vkEnsure(vkGetPhysicalDeviceSurfaceFormatsKHR(this->_physicalDevice, this->_surface, &count,
                                                  surfaceFormats.data()));

    return surfaceFormats;
}

VkSurfaceFormatKHR PhysicalDevice::getPreferredSurfaceFormat() {
    std::vector<VkSurfaceFormatKHR> formats = getSurfaceFormats();

    auto it = std::find_if(formats.begin(), formats.end(), [](const VkSurfaceFormatKHR &format) {
        return format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR &&
               format.format == VK_FORMAT_B8G8R8A8_SRGB;
    });

    return it != formats.end()
           ? *it
           : formats[0];
}

VkPresentModeKHR PhysicalDevice::getPreferredPresentMode() {
#ifdef VSYNC
    std::vector<VkPresentModeKHR> presentModes = getPresentModes();

    auto it = std::find(presentModes.begin(), presentModes.end(), VK_PRESENT_MODE_MAILBOX_KHR);

    return it != presentModes.end()
           ? *it
           : VK_PRESENT_MODE_FIFO_KHR;
#else
    return VK_PRESENT_MODE_IMMEDIATE_KHR;
#endif
}

VkFormat PhysicalDevice::getColorFormat() {
    return getPreferredSurfaceFormat().format;
}

VkFormat PhysicalDevice::getDepthFormat() {
    for (VkFormat depthFormat: PREFERRED_DEPTH_FORMATS) {
        VkFormatProperties formatProps;
        vkGetPhysicalDeviceFormatProperties(this->_physicalDevice, depthFormat, &formatProps);

        if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            return depthFormat;
        }
    }

    throw std::runtime_error("No depth format available");
}

VkSampleCountFlagBits PhysicalDevice::getMsaaSamples() {
    VkPhysicalDeviceProperties properties = getProperties();

    VkSampleCountFlags counts = properties.limits.framebufferColorSampleCounts &
                                properties.limits.framebufferDepthSampleCounts;

    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

    return VK_SAMPLE_COUNT_1_BIT;
}

float PhysicalDevice::getMaxSamplerAnisotropy() {
    return getProperties().limits.maxSamplerAnisotropy;
}

uint32_t PhysicalDevice::getSuitableMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags memoryProperty) {
    VkPhysicalDeviceMemoryProperties properties;
    vkGetPhysicalDeviceMemoryProperties(this->_physicalDevice, &properties);

    for (uint32_t idx = 0; idx < properties.memoryTypeCount; idx++) {
        bool memoryTypeMatches = memoryTypeBits & (1 << idx);
        bool memoryPropertyMatches = (properties.memoryTypes[idx].propertyFlags & memoryProperty) == memoryProperty;

        if (!memoryTypeMatches || !memoryPropertyMatches) {
            continue;
        }

        return idx;
    }

    throw std::runtime_error("No suitable memory type available");
}

std::shared_ptr<PhysicalDevice> PhysicalDevice::selectSuitable(VkInstance instance, VkSurfaceKHR surface) {
    uint32_t count;
    vkEnsure(vkEnumeratePhysicalDevices(instance, &count, nullptr));

    if (count == 0) {
        throw std::runtime_error("No physical device available");
    }

    std::vector<VkPhysicalDevice> physicalDevices(count);
    vkEnsure(vkEnumeratePhysicalDevices(instance, &count, physicalDevices.data()));

    for (VkPhysicalDevice physicalDevice: physicalDevices) {
        QueueFamilies queueFamilies = getQueueFamilies(physicalDevice, surface);

        if (!queueFamilies.isCompleted()) {
            continue;
        }

        std::vector<std::string> extensions = getExtensions(physicalDevice);
        bool requiredExtensionsSupported = std::all_of(
                DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end(),
                [extensions](const char *requiredExtension) {
                    return std::find(extensions.begin(), extensions.end(), requiredExtension) != extensions.end();
                });

        if (!requiredExtensionsSupported) {
            continue;
        }

        return std::make_shared<PhysicalDevice>(physicalDevice, surface, queueFamilies.graphicsIdx.value(),
                                                queueFamilies.presentIdx.value());
    }

    throw std::runtime_error("No supported physical device available");
}
