#include "CommandBufferProxy.hpp"

#include "src/Rendering/Proxies/LogicalDeviceProxy.hpp"

CommandBufferProxy::CommandBufferProxy(const std::shared_ptr<LogicalDeviceProxy> &logicalDevice,
                                       const vk::CommandPool &commandPool,
                                       const vk::CommandBuffer &handle)
        : _logicalDevice(logicalDevice),
          _commandPool(commandPool),
          _handle(handle) {
    //
}

void CommandBufferProxy::reset() {
    this->_handle.reset();
}

void CommandBufferProxy::destroy() {
    this->_logicalDevice->getHandle().freeCommandBuffers(this->_commandPool, {this->_handle});
}
