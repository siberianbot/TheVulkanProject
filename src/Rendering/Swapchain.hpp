#ifndef RENDERING_SWAPCHAIN_HPP
#define RENDERING_SWAPCHAIN_HPP

#include <memory>
#include <vector>

#include <vulkan/vulkan.hpp>

class PhysicalDevice;
class RenderingDevice;
class VulkanObjectsAllocator;
class ImageViewObject;

class Swapchain {
private:
    std::shared_ptr<PhysicalDevice> _physicalDevice;
    std::shared_ptr<RenderingDevice> _renderingDevice;
    std::shared_ptr<VulkanObjectsAllocator> _vulkanObjectsAllocator;

    VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
    VkExtent2D _swapchainExtent = {};
    std::vector<VkImage> _swapchainImages;
    std::vector<std::shared_ptr<ImageViewObject>> _swapchainImageViews;
    uint32_t _swapchainMinImageCount;

public:
    Swapchain(const std::shared_ptr<PhysicalDevice> &physicalDevice,
              const std::shared_ptr<RenderingDevice> &renderingDevice,
              const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator);

    [[nodiscard]] VkSwapchainKHR getHandle() const { return this->_swapchain; }

    [[nodiscard]] VkExtent2D getSwapchainExtent() const { return this->_swapchainExtent; }
    [[nodiscard]] uint32_t getImageCount() const { return this->_swapchainImages.size(); }
    [[nodiscard]] uint32_t getMinImageCount() const { return this->_swapchainMinImageCount; }

    [[nodiscard]] std::shared_ptr<ImageViewObject> getSwapchainImageView(uint32_t imageIdx) const {
        return this->_swapchainImageViews[imageIdx];
    }

    void create();
    void destroy();
};


#endif // RENDERING_SWAPCHAIN_HPP
