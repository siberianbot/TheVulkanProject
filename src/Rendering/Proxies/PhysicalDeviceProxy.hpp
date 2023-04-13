#ifndef RENDERING_PROXIES_PHYSICALDEVICEPROXY_HPP
#define RENDERING_PROXIES_PHYSICALDEVICEPROXY_HPP

#include <optional>

#include <vulkan/vulkan.hpp>

struct PhysicalDeviceSupportInfo {
    uint32_t graphicsQueueFamilyIdx;
    uint32_t presentQueueFamilyIdx;
};

class PhysicalDeviceProxy {
private:
    vk::PhysicalDevice _handle;
    vk::PhysicalDeviceProperties _properties;
    uint32_t _graphicsQueueFamilyIdx;
    uint32_t _presentQueueFamilyIdx;

public:
    PhysicalDeviceProxy(const vk::PhysicalDevice &handle,
                        const vk::PhysicalDeviceProperties &properties,
                        const PhysicalDeviceSupportInfo &supportInfo);

    [[nodiscard]] const vk::PhysicalDevice &getHandle() const { return this->_handle; }

    [[nodiscard]] const vk::PhysicalDeviceProperties &getProperties() const { return this->_properties; }

    [[nodiscard]] const uint32_t &getGraphicsQueueFamilyIdx() const { return this->_graphicsQueueFamilyIdx; }

    [[nodiscard]] const uint32_t &getPresentQueueFamilyIdx() const { return this->_presentQueueFamilyIdx; }

    [[nodiscard]] static std::optional<PhysicalDeviceSupportInfo> getSupportInfoFor(
            const vk::PhysicalDevice &physicalDevice,
            const vk::SurfaceKHR &surface);
};

#endif // RENDERING_PROXIES_PHYSICALDEVICEPROXY_HPP
