#include "Swapchain.hpp"

#include <limits>
#include <string_view>

#include "src/Engine/EngineError.hpp"
#include "src/Engine/Log.hpp"
#include "src/Engine/VarCollection.hpp"
#include "src/Engine/Vars.hpp"
#include "src/Rendering/SurfaceManager.hpp"
#include "src/Rendering/Proxies/LogicalDeviceProxy.hpp"
#include "src/Rendering/Proxies/PhysicalDeviceProxy.hpp"
#include "src/System/Window.hpp"

static constexpr const std::string_view SWAPCHAIN_TAG = "Swapchain";

static constexpr const std::array<vk::PresentModeKHR, 2> PREFERRED_PRESENT_MODES = {
        vk::PresentModeKHR::eMailbox,
        vk::PresentModeKHR::eFifoRelaxed
};

vk::SurfaceFormatKHR Swapchain::getPreferredSurfaceFormatFor(const vk::SurfaceKHR &surface) {
    std::vector<vk::SurfaceFormatKHR> formats;
    try {
        formats = this->_physicalDevice->getHandle().getSurfaceFormatsKHR(surface);
    } catch (const std::exception &error) {
        this->_log->error(SWAPCHAIN_TAG, error);
        throw EngineError("Failed to retrieve surface formats");
    }

    auto it = std::find_if(formats.begin(), formats.end(),
                           [](const vk::SurfaceFormatKHR &format) {
                               return format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear &&
                                      format.format == vk::Format::eB8G8R8A8Srgb;
                           });

    if (it != formats.end()) {
        return *it;
    }

    return formats[0];
}

vk::PresentModeKHR Swapchain::getPreferredPresentModeFor(const vk::SurfaceKHR &surface) {
    std::vector<vk::PresentModeKHR> modes;
    try {
        modes = this->_physicalDevice->getHandle().getSurfacePresentModesKHR(surface);
    } catch (const std::exception &error) {
        this->_log->error(SWAPCHAIN_TAG, error);
        throw EngineError("Failed to retrieve surface present modes");
    }

    if (!this->_varCollection->getOrDefault(RENDERING_VSYNC.data(), false)) {
        return vk::PresentModeKHR::eImmediate;
    }

    for (const auto &preferredMode: PREFERRED_PRESENT_MODES) {
        if (std::find(modes.begin(), modes.end(), preferredMode) != modes.end()) {
            return preferredMode;
        }
    }

    return vk::PresentModeKHR::eImmediate;
}

Swapchain::Swapchain(const std::shared_ptr<Log> &log,
                     const std::shared_ptr<VarCollection> &varCollection,
                     const std::shared_ptr<SurfaceManager> &surfaceManager,
                     const std::shared_ptr<PhysicalDeviceProxy> &physicalDevice,
                     const std::shared_ptr<LogicalDeviceProxy> &logicalDevice,
                     const std::shared_ptr<Window> &window)
        : _log(log),
          _varCollection(varCollection),
          _surfaceManager(surfaceManager),
          _physicalDevice(physicalDevice),
          _logicalDevice(logicalDevice),
          _window(window) {
    //
}

void Swapchain::create() {
    auto surface = this->_surfaceManager->getSurfaceFor(this->_window->handle());

    vk::SurfaceCapabilitiesKHR capabilities;
    try {
        capabilities = this->_physicalDevice->getHandle().getSurfaceCapabilitiesKHR(surface);
    } catch (const std::exception &error) {
        this->_log->error(SWAPCHAIN_TAG, error);
        throw EngineError("Failed to retrieve surface capabilities");
    }

    vk::Extent2D extent = vk::Extent2D(
            capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()
            ? capabilities.currentExtent.width
            : this->_window->width(),
            capabilities.currentExtent.height != std::numeric_limits<uint32_t>::max()
            ? capabilities.currentExtent.height
            : this->_window->height());

    uint32_t imageCount = std::clamp(capabilities.minImageCount + 1,
                                     capabilities.minImageCount,
                                     capabilities.maxImageCount);

    vk::SurfaceFormatKHR surfaceFormat = this->getPreferredSurfaceFormatFor(surface);

    vk::PresentModeKHR presentMode = this->getPreferredPresentModeFor(surface);

    auto createInfo = vk::SwapchainCreateInfoKHR()
            .setSurface(surface)
            .setMinImageCount(imageCount)
            .setImageFormat(surfaceFormat.format)
            .setImageColorSpace(surfaceFormat.colorSpace)
            .setImageExtent(extent)
            .setImageArrayLayers(1)
            .setPresentMode(presentMode)
            .setPreTransform(capabilities.currentTransform)
            .setClipped(true)
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);

    if (this->_swapchain.has_value()) {
        createInfo.setOldSwapchain(this->_swapchain.value());
    }

    vk::SwapchainKHR swapchain;
    try {
        swapchain = this->_logicalDevice->getHandle().createSwapchainKHR(createInfo);
    } catch (const std::exception &error) {
        this->_log->error(SWAPCHAIN_TAG, error);
        throw EngineError("Failed to create swapchain");
    }

    if (this->_swapchain.has_value()) {
        this->destroy();
    }

    this->_swapchain = swapchain;
    this->_swapchainColorFormat = surfaceFormat.format;
    this->_swapchainExtent = extent;
    this->_swapchainMinImageCount = capabilities.minImageCount;
    this->_swapchainImageCount = imageCount;

    std::vector<vk::Image> images;
    try {
        images = this->_logicalDevice->getHandle().getSwapchainImagesKHR(this->_swapchain.value());
    } catch (const std::exception &error) {
        this->_log->error(SWAPCHAIN_TAG, error);
        throw EngineError("Failed to retrieve swapchain images");
    }

    auto imageSubresourceRange = vk::ImageSubresourceRange()
            .setAspectMask(vk::ImageAspectFlagBits::eColor)
            .setBaseMipLevel(0)
            .setLevelCount(1)
            .setBaseArrayLayer(0)
            .setLayerCount(1);

    auto imageViewCreateInfo = vk::ImageViewCreateInfo()
            .setFormat(surfaceFormat.format)
            .setViewType(vk::ImageViewType::e2D)
            .setSubresourceRange(imageSubresourceRange);

    std::vector<vk::ImageView> imageViews;
    for (const auto &image: images) {
        imageViewCreateInfo.setImage(image);

        vk::ImageView imageView;
        try {
            imageView = this->_logicalDevice->getHandle().createImageView(imageViewCreateInfo);
        } catch (const std::exception &error) {
            this->_log->error(SWAPCHAIN_TAG, error);
            throw EngineError("Failed to create swapchain image view");
        }

        imageViews.push_back(imageView);
    }

    this->_swapchainImageViews = imageViews;
}

void Swapchain::destroy() {
    if (!this->_swapchain.has_value()) {
        return;
    }

    this->_logicalDevice->getHandle().waitIdle();

    for (const auto &imageView: this->_swapchainImageViews) {
        this->_logicalDevice->getHandle().destroy(imageView);
    }

    this->_logicalDevice->getHandle().destroy(this->_swapchain.value());

    this->_swapchain = std::nullopt;
    this->_swapchainColorFormat = std::nullopt;
    this->_swapchainExtent = std::nullopt;
    this->_swapchainMinImageCount = std::nullopt;
    this->_swapchainImageCount = std::nullopt;
    this->_swapchainImageViews.clear();
}
