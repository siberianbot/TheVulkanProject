#include "LogicalDeviceProxy.hpp"

LogicalDeviceProxy::LogicalDeviceProxy(const vk::Device &handle,
                                       const vk::Queue &graphicsQueue,
                                       const vk::Queue &presentQueue)
        : _handle(handle),
          _graphicsQueue(graphicsQueue),
          _presentQueue(presentQueue) {
    //
}

void LogicalDeviceProxy::destroy() {
    this->_handle.destroy();
}
