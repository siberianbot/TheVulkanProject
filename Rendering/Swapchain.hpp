#ifndef RENDERING_SWAPCHAIN_HPP
#define RENDERING_SWAPCHAIN_HPP

#include <vector>

#include <vulkan/vulkan.hpp>

#include "Rendering/Objects/ImageObject.hpp"

class RenderingDevice;
class RenderingObjectsFactory;

class Swapchain {
private:
    RenderingDevice *_renderingDevice;
    RenderingObjectsFactory *_renderingObjectsFactory;

    VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
    VkExtent2D _swapchainExtent = {};
    std::vector<ImageObject *> _swapchainImages;

    ImageObject *_colorImage = nullptr;
    ImageObject *_depthImage = nullptr;

public:
    explicit Swapchain(RenderingDevice *renderingDevice, RenderingObjectsFactory *renderingObjectsFactory);
    ~Swapchain();

    [[deprecated]] [[nodiscard]] VkSwapchainKHR getHandle() const { return this->_swapchain; }

    [[nodiscard]] VkExtent2D getSwapchainExtent() const { return this->_swapchainExtent; }
    [[nodiscard]] uint32_t getImageCount() const { return this->_swapchainImages.size(); }

    [[nodiscard]] ImageObject *getColorImage() const { return this->_colorImage; }
    [[nodiscard]] ImageObject *getDepthImage() const { return this->_depthImage; }
    [[nodiscard]] ImageObject *getSwapchainImage(uint32_t imageIdx) const { return this->_swapchainImages[imageIdx]; }

    void create();
    void destroy();
};


#endif // RENDERING_SWAPCHAIN_HPP
