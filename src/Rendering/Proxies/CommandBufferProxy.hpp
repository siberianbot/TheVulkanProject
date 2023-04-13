#ifndef RENDERING_PROXIES_COMMANDBUFFERPROXY_HPP
#define RENDERING_PROXIES_COMMANDBUFFERPROXY_HPP

#include <memory>

#include <vulkan/vulkan.hpp>

class LogicalDeviceProxy;

class CommandBufferProxy {
private:
    std::shared_ptr<LogicalDeviceProxy> _logicalDevice;
    vk::CommandPool _commandPool;
    vk::CommandBuffer _handle;

public:
    CommandBufferProxy(const std::shared_ptr<LogicalDeviceProxy> &logicalDevice,
                       const vk::CommandPool &commandPool,
                       const vk::CommandBuffer &handle);

    void reset();

    void destroy();

    [[nodiscard]] const vk::CommandBuffer &getHandle() const { return this->_handle; }
};

#endif // RENDERING_PROXIES_COMMANDBUFFERPROXY_HPP
