#include "PhysicalDeviceProxy.hpp"

#include "src/Rendering/Extensions.hpp"

PhysicalDeviceProxy::PhysicalDeviceProxy(const vk::PhysicalDevice &handle,
                                         const vk::PhysicalDeviceProperties &properties,
                                         const PhysicalDeviceSupportInfo &supportInfo)
        : _handle(handle),
          _properties(properties),
          _graphicsQueueFamilyIdx(supportInfo.graphicsQueueFamilyIdx),
          _presentQueueFamilyIdx(supportInfo.presentQueueFamilyIdx) {
    //
}

std::optional<PhysicalDeviceSupportInfo> PhysicalDeviceProxy::getSupportInfoFor(
        const vk::PhysicalDevice &physicalDevice,
        const vk::SurfaceKHR &surface) {

    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
    std::optional<uint32_t> graphicsIdx;
    std::optional<uint32_t> presentIdx;

    for (uint32_t idx = 0; idx < queueFamilyProperties.size(); idx++) {
        if (graphicsIdx.has_value() && presentIdx.has_value()) {
            break;
        }

        if (queueFamilyProperties[idx].queueFlags & vk::QueueFlagBits::eGraphics) {
            graphicsIdx = idx;
        }

        if (physicalDevice.getSurfaceSupportKHR(idx, surface)) {
            presentIdx = idx;
        }
    }

    if (!graphicsIdx.has_value() || !presentIdx.has_value()) {
        return std::nullopt;
    }

    std::vector<vk::ExtensionProperties> extensionProperties = physicalDevice.enumerateDeviceExtensionProperties();

    if (std::any_of(REQUIRED_DEVICE_EXTENSIONS.begin(), REQUIRED_DEVICE_EXTENSIONS.end(),
                    [&extensionProperties](const char *extensionName) -> bool {
                        return std::find_if(extensionProperties.begin(), extensionProperties.end(),
                                            [&extensionName](const vk::ExtensionProperties &extension) -> bool {
                                                return extension.extensionName == extensionName;
                                            }) == extensionProperties.end();
                    })) {
        return std::nullopt;
    }

    return PhysicalDeviceSupportInfo{
            .graphicsQueueFamilyIdx = graphicsIdx.value(),
            .presentQueueFamilyIdx = presentIdx.value()
    };
}
