#include "Swapchain.hpp"

#include "PhysicalDevice.hpp"
#include "RenderingDevice.hpp"
#include "RenderingObjectsFactory.hpp"
#include "src/Rendering/Objects/ImageObject.hpp"
#include "src/Rendering/Objects/ImageViewObject.hpp"

Swapchain::Swapchain(RenderingDevice *renderingDevice, RenderingObjectsFactory *renderingObjectsFactory)
        : _renderingDevice(renderingDevice),
          _renderingObjectsFactory(renderingObjectsFactory) {
    //
}

Swapchain::~Swapchain() {
    this->destroy();
}

void Swapchain::create() {
    if (this->_swapchain != VK_NULL_HANDLE) {
        destroy();
    }

    VkSurfaceCapabilitiesKHR capabilities = this->_renderingDevice->getPhysicalDevice()->getSurfaceCapabilities();

    // TODO:
    //  capabilities.currentExtent may be not available on certain platform;
    //  future investigations required
    this->_swapchainExtent = capabilities.currentExtent;
    this->_swapchainMinImageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 &&
        this->_swapchainMinImageCount > capabilities.maxImageCount) {
        this->_swapchainMinImageCount = capabilities.maxImageCount;
    }

    this->_swapchain = this->_renderingDevice->createSwapchain(this->_swapchainExtent, this->_swapchainMinImageCount);
    this->_swapchainImages = this->_renderingDevice->getSwapchainImages(this->_swapchain);
    this->_swapchainImageViews.resize(this->_swapchainImages.size());

    for (uint32_t idx = 0; idx < this->_swapchainImages.size(); idx++) {
        this->_swapchainImageViews[idx] = this->_renderingObjectsFactory->createImageViewObject(
                this->_swapchainImages[idx],
                this->_renderingDevice->getPhysicalDevice()->getColorFormat(),
                VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

void Swapchain::destroy() {
    if (this->_swapchain == VK_NULL_HANDLE) {
        return;
    }

    for (ImageViewObject *imageView: this->_swapchainImageViews) {
        delete imageView;
    }

    this->_renderingDevice->destroySwapchain(this->_swapchain);
    this->_swapchain = VK_NULL_HANDLE;
}
