#include "Swapchain.hpp"

#include "src/Rendering/PhysicalDevice.hpp"
#include "src/Rendering/RenderingDevice.hpp"
#include "src/Rendering/VulkanObjectsAllocator.hpp"
#include "src/Rendering/Builders/ImageViewObjectBuilder.hpp"
#include "src/Rendering/Objects/ImageViewObject.hpp"

Swapchain::Swapchain(const std::shared_ptr<PhysicalDevice> &physicalDevice,
                     const std::shared_ptr<RenderingDevice> &renderingDevice,
                     const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator)
        : _physicalDevice(physicalDevice),
          _renderingDevice(renderingDevice),
          _vulkanObjectsAllocator(vulkanObjectsAllocator) {
    //
}

void Swapchain::create() {
    if (this->_swapchain != VK_NULL_HANDLE) {
        destroy();
    }

    VkSurfaceCapabilitiesKHR capabilities = this->_physicalDevice->getSurfaceCapabilities();

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

    VkFormat colorFormat = this->_physicalDevice->getColorFormat();

    for (uint32_t idx = 0; idx < this->_swapchainImages.size(); idx++) {
        this->_swapchainImageViews[idx] = ImageViewObjectBuilder(this->_vulkanObjectsAllocator)
                .withImage(this->_swapchainImages[idx])
                .withFormat(colorFormat)
                .withAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT)
                .build();
    }
}

void Swapchain::destroy() {
    if (this->_swapchain == VK_NULL_HANDLE) {
        return;
    }

    for (std::shared_ptr<ImageViewObject> imageView: this->_swapchainImageViews) {
        imageView->destroy();
    }

    this->_swapchainImageViews.clear();

    this->_renderingDevice->destroySwapchain(this->_swapchain);
    this->_swapchain = VK_NULL_HANDLE;
}
