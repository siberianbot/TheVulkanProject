#ifndef RENDERING_SWAPCHAIN_HPP
#define RENDERING_SWAPCHAIN_HPP

#include <vector>

#include <vulkan/vulkan.hpp>

class RenderingDevice;
class RenderingObjectsFactory;
class ImageObject;
class ImageViewObject;

class Swapchain {
private:
    RenderingDevice *_renderingDevice;
    RenderingObjectsFactory *_renderingObjectsFactory;

    VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
    VkExtent2D _swapchainExtent = {};
    std::vector<VkImage> _swapchainImages;
    std::vector<ImageViewObject *> _swapchainImageViews;
    uint32_t _swapchainMinImageCount;

    ImageObject *_colorImage = nullptr;
    ImageViewObject *_colorImageView = nullptr;
    ImageObject *_depthImage = nullptr;
    ImageViewObject *_depthImageView = nullptr;

public:
    explicit Swapchain(RenderingDevice *renderingDevice, RenderingObjectsFactory *renderingObjectsFactory);
    ~Swapchain();

    [[nodiscard]] VkSwapchainKHR getHandle() const { return this->_swapchain; }

    [[nodiscard]] VkExtent2D getSwapchainExtent() const { return this->_swapchainExtent; }
    [[nodiscard]] uint32_t getImageCount() const { return this->_swapchainImages.size(); }
    [[nodiscard]] uint32_t getMinImageCount() const { return this->_swapchainMinImageCount; }

    [[nodiscard]] ImageViewObject *getColorImageView() const { return this->_colorImageView; }
    [[nodiscard]] ImageViewObject *getDepthImageView() const { return this->_depthImageView; }
    [[nodiscard]] ImageViewObject *getSwapchainImageView(uint32_t imageIdx) const {
        return this->_swapchainImageViews[imageIdx];
    }

    void create();
    void destroy();
};


#endif // RENDERING_SWAPCHAIN_HPP
