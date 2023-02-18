#ifndef RENDERING_VULKANPHYSICALDEVICE_HPP
#define RENDERING_VULKANPHYSICALDEVICE_HPP

#include <optional>

#include <vulkan/vulkan.hpp>

#include "Rendering/RenderingDevice.hpp"

// TODO naming - remove Vulkan prefix
class VulkanPhysicalDevice {
private:
    struct QueueFamilies {
        std::optional<uint32_t> graphicsIdx;
        std::optional<uint32_t> presentIdx;

        bool isCompleted() {
            return graphicsIdx.has_value() && presentIdx.has_value();
        }
    };

    VkPhysicalDevice _physicalDevice;
    VkSurfaceKHR _surface;
    uint32_t _graphicsQueueFamilyIdx;
    uint32_t _presentQueueFamilyIdx;

    explicit VulkanPhysicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                  uint32_t graphicsQueueFamilyIdx, uint32_t presentQueueFamilyIdx);

    static QueueFamilies getQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
    static std::vector<std::string> getExtensions(VkPhysicalDevice physicalDevice);

public:
    [[deprecated]] [[nodiscard]] VkPhysicalDevice getHandle() const { return this->_physicalDevice; }

    [[nodiscard]] uint32_t getGraphicsQueueFamilyIdx() const { return this->_graphicsQueueFamilyIdx; }
    [[nodiscard]] uint32_t getPresentQueueFamilyIdx() const { return this->_presentQueueFamilyIdx; }

    VkPhysicalDeviceProperties getProperties();
    VkSurfaceCapabilitiesKHR getSurfaceCapabilities();
    std::vector<VkPresentModeKHR> getPresentModes();
    std::vector<VkSurfaceFormatKHR> getSurfaceFormats();

    VkSurfaceFormatKHR getPreferredSurfaceFormat();
    VkPresentModeKHR getPreferredPresentMode();

    VkFormat getColorFormat();
    VkFormat getDepthFormat();
    VkSampleCountFlagBits getMsaaSamples();
    float getMaxSamplerAnisotropy();

    uint32_t getSuitableMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags memoryProperty);

    RenderingDevice *createRenderingDevice();

    static VulkanPhysicalDevice *selectSuitable(VkInstance instance, VkSurfaceKHR surface);
};

#endif // RENDERING_VULKANPHYSICALDEVICE_HPP
