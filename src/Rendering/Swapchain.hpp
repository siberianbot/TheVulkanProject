#ifndef RENDERING_SWAPCHAIN_HPP
#define RENDERING_SWAPCHAIN_HPP

#include <memory>
#include <optional>
#include <thread>
#include <vector>

#include <vulkan/vulkan.hpp>

class Log;
class VarCollection;
class EventQueue;
class SurfaceManager;
class PhysicalDeviceProxy;
class LogicalDeviceProxy;
class Window;

class Swapchain {
    std::shared_ptr<Log> _log;
    std::shared_ptr<VarCollection> _varCollection;
    std::shared_ptr<SurfaceManager> _surfaceManager;
    std::shared_ptr<PhysicalDeviceProxy> _physicalDevice;
    std::shared_ptr<LogicalDeviceProxy> _logicalDevice;
    std::shared_ptr<Window> _window;

    bool _invalid;
    std::optional<vk::SwapchainKHR> _swapchain;
    std::optional<vk::Format> _swapchainColorFormat;
    std::optional<vk::Extent2D> _swapchainExtent;
    std::optional<uint32_t> _swapchainMinImageCount;
    std::optional<uint32_t> _swapchainImageCount;
    std::vector<vk::ImageView> _swapchainImageViews;

    vk::SurfaceFormatKHR getPreferredSurfaceFormatFor(const vk::SurfaceKHR &surface);
    vk::PresentModeKHR getPreferredPresentModeFor(const vk::SurfaceKHR &surface);

public:
    Swapchain(const std::shared_ptr<Log> &log,
              const std::shared_ptr<VarCollection> &varCollection,
              const std::shared_ptr<SurfaceManager> &surfaceManager,
              const std::shared_ptr<PhysicalDeviceProxy> &physicalDevice,
              const std::shared_ptr<LogicalDeviceProxy> &logicalDevice,
              const std::shared_ptr<Window> &window);

    void create();
    void destroy();

    void invalidate();

    [[nodiscard]] std::optional<uint32_t> acquireNextImage(const vk::Semaphore &semaphore);

    [[nodiscard]] const bool &isInvalid() const { return this->_invalid; }

    [[nodiscard]] const vk::SwapchainKHR &getHandle() const { return this->_swapchain.value(); }

    [[nodiscard]] const vk::Format &getColorFormat() const { return this->_swapchainColorFormat.value(); }

    [[nodiscard]] const vk::Extent2D &getExtent() const { return this->_swapchainExtent.value(); }

    [[nodiscard]] const uint32_t &getMinImageCount() const { return this->_swapchainMinImageCount.value(); }

    [[nodiscard]] const uint32_t &getImageCount() const { return this->_swapchainImageCount.value(); }

    [[nodiscard]] const std::vector<vk::ImageView> &getImageViews() const { return this->_swapchainImageViews; }
};

#endif // RENDERING_SWAPCHAIN_HPP
