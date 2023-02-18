#include "RenderingDevice.hpp"

RenderingDevice::RenderingDevice(VulkanPhysicalDevice *physicalDevice, VkDevice device,
                                 VkQueue graphicsQueue, VkQueue presentQueue)
        : _physicalDevice(physicalDevice),
          _device(device),
          _graphicsQueue(graphicsQueue),
          _presentQueue(presentQueue) {
    //
}

RenderingDevice::~RenderingDevice() {
    vkDestroyDevice(this->_device, nullptr);
}

void RenderingDevice::waitIdle() {
    vkDeviceWaitIdle(this->_device);
}
