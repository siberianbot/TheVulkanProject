#ifndef RENDERING_PROXIES_LOGICALDEVICEPROXY_HPP
#define RENDERING_PROXIES_LOGICALDEVICEPROXY_HPP

#include <vulkan/vulkan.hpp>

class LogicalDeviceProxy {
private:
    vk::Device _handle;
    vk::Queue _graphicsQueue;
    vk::Queue _presentQueue;

public:
    LogicalDeviceProxy(const vk::Device &handle,
                       const vk::Queue &graphicsQueue,
                       const vk::Queue &presentQueue);

    void destroy();

    [[nodiscard]] const vk::Device &getHandle() const { return this->_handle; }

    [[nodiscard]] const vk::Queue &getGraphicsQueue() const { return this->_graphicsQueue; }

    [[nodiscard]] const vk::Queue &getPresentQueue() const { return this->_presentQueue; }
};


#endif // RENDERING_PROXIES_LOGICALDEVICEPROXY_HPP
