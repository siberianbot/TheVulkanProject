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
    VkFormat colorFormat = this->_renderingDevice->getPhysicalDevice()->getColorFormat();
    VkFormat depthFormat = this->_renderingDevice->getPhysicalDevice()->getDepthFormat();
    VkSampleCountFlagBits samples = this->_renderingDevice->getPhysicalDevice()->getMsaaSamples();

    this->_swapchainImageViews.resize(this->_swapchainImages.size());

    for (uint32_t idx = 0; idx < this->_swapchainImages.size(); idx++) {
        this->_swapchainImageViews[idx] = this->_renderingObjectsFactory->createImageViewObject(
                this->_swapchainImages[idx], colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    this->_colorImage = this->_renderingObjectsFactory->createImageObject(this->_swapchainExtent.width,
                                                                          this->_swapchainExtent.height,
                                                                          colorFormat,
                                                                          VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                                                                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                                                          samples,
                                                                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    this->_colorImageView = this->_renderingObjectsFactory->createImageViewObject(this->_colorImage,
                                                                                  VK_IMAGE_ASPECT_COLOR_BIT);

    this->_depthImage = this->_renderingObjectsFactory->createImageObject(this->_swapchainExtent.width,
                                                                          this->_swapchainExtent.height,
                                                                          depthFormat,
                                                                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                                                          samples,
                                                                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    this->_depthImageView = this->_renderingObjectsFactory->createImageViewObject(this->_depthImage,
                                                                                  VK_IMAGE_ASPECT_DEPTH_BIT);
}

void Swapchain::destroy() {
    if (this->_swapchain == VK_NULL_HANDLE) {
        return;
    }

    delete this->_colorImageView;
    delete this->_depthImageView;
    delete this->_colorImage;
    delete this->_depthImage;

    for (ImageViewObject *imageView: this->_swapchainImageViews) {
        delete imageView;
    }

    this->_renderingDevice->destroySwapchain(this->_swapchain);
    this->_swapchain = VK_NULL_HANDLE;
}
