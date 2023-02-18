#ifndef RENDERING_RENDERINGDEVICE_HPP
#define RENDERING_RENDERINGDEVICE_HPP

#include <vulkan/vulkan.hpp>

class VulkanPhysicalDevice;

class RenderingDevice {
private:
    VulkanPhysicalDevice *_physicalDevice;
    VkDevice _device;
    VkQueue _graphicsQueue;
    VkQueue _presentQueue;

public:
    RenderingDevice(VulkanPhysicalDevice *physicalDevice, VkDevice device,
                    VkQueue graphicsQueue, VkQueue presentQueue);
    ~RenderingDevice();

    [[deprecated]] [[nodiscard]] VkDevice getHandle() const { return this->_device; }

    [[nodiscard]] VulkanPhysicalDevice *getPhysicalDevice() const { return this->_physicalDevice; }

    [[nodiscard]] VkQueue getGraphicsQueue() const { return this->_graphicsQueue; }
    [[nodiscard]] VkQueue getPresentQueue() const { return this->_presentQueue; }

    void waitIdle();
};

#endif // RENDERING_RENDERINGDEVICE_HPP
