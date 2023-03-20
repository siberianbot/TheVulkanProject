#ifndef RENDERING_PHYSICALDEVICE_HPP
#define RENDERING_PHYSICALDEVICE_HPP

#include <memory>
#include <optional>

#include <vulkan/vulkan.hpp>

class PhysicalDevice {
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

    static QueueFamilies getQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
    static std::vector<std::string> getExtensions(VkPhysicalDevice physicalDevice);

public:
    PhysicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                   uint32_t graphicsQueueFamilyIdx, uint32_t presentQueueFamilyIdx);

    [[nodiscard]] VkPhysicalDevice getHandle() const { return this->_physicalDevice; }
    [[nodiscard]] VkSurfaceKHR getSurface() const { return this->_surface; }

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

    static std::shared_ptr<PhysicalDevice> selectSuitable(VkInstance instance, VkSurfaceKHR surface);
};

#endif // RENDERING_PHYSICALDEVICE_HPP
