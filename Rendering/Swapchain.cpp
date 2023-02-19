#include "Swapchain.hpp"

#include "VulkanCommon.hpp"
#include "Rendering/VulkanPhysicalDevice.hpp"
#include "Rendering/RenderingDevice.hpp"
#include "Rendering/RenderingObjectsFactory.hpp"

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

    // TODO:
    //  capabilities.currentExtent may be not available on certain platform;
    //  future investigations required
    this->_swapchainExtent = this->_renderingDevice->getPhysicalDevice()->getSurfaceCapabilities().currentExtent;
    this->_swapchain = this->_renderingDevice->createSwapchain(this->_swapchainExtent);

    std::vector<VkImage> images = this->_renderingDevice->getSwapchainImages(this->_swapchain);
    VkFormat colorFormat = this->_renderingDevice->getPhysicalDevice()->getColorFormat();
    VkFormat depthFormat = this->_renderingDevice->getPhysicalDevice()->getDepthFormat();
    VkSampleCountFlagBits samples = this->_renderingDevice->getPhysicalDevice()->getMsaaSamples();

    this->_swapchainImages.resize(images.size());

    for (uint32_t idx = 0; idx < images.size(); idx++) {
        VkImage image = images[idx];
        VkImageView imageView = this->_renderingDevice->createImageView(image, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);

        this->_swapchainImages[idx] = new ImageObject(this->_renderingDevice, image, imageView);
    }

    this->_colorImage = this->_renderingObjectsFactory->createImageObject(this->_swapchainExtent.width,
                                                                          this->_swapchainExtent.height,
                                                                          colorFormat,
                                                                          VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                                                                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                                                          samples,
                                                                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                                          VK_IMAGE_ASPECT_COLOR_BIT);

    this->_depthImage = this->_renderingObjectsFactory->createImageObject(this->_swapchainExtent.width,
                                                                          this->_swapchainExtent.height,
                                                                          depthFormat,
                                                                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                                                          samples,
                                                                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                                          VK_IMAGE_ASPECT_DEPTH_BIT);
}

void Swapchain::destroy() {
    if (this->_swapchain == VK_NULL_HANDLE) {
        return;
    }

    delete this->_colorImage;
    delete this->_depthImage;

    for (ImageObject *image: this->_swapchainImages) {
        delete image;
    }

    this->_renderingDevice->destroySwapchain(this->_swapchain);
    this->_swapchain = VK_NULL_HANDLE;
}
