#ifndef RENDERING_SWAPCHAINMANAGER_HPP
#define RENDERING_SWAPCHAINMANAGER_HPP

#include <map>
#include <memory>

#include <vulkan/vulkan.hpp>

#include "src/Events/EventHandlerIdx.hpp"

class Log;
class VarCollection;
class EventQueue;
class SurfaceManager;
class PhysicalDeviceProxy;
class LogicalDeviceProxy;
class Window;

class Swapchain;

class SwapchainManager {
private:
    std::shared_ptr<Log> _log;
    std::shared_ptr<VarCollection> _varCollection;
    std::shared_ptr<EventQueue> _eventQueue;
    std::shared_ptr<SurfaceManager> _surfaceManager;
    std::shared_ptr<PhysicalDeviceProxy> _physicalDevice;
    std::shared_ptr<LogicalDeviceProxy> _logicalDevice;

    EventHandlerIdx _handlerIdx;
    std::map<Window *, std::shared_ptr<Swapchain>> _swapchains;

public:
    SwapchainManager(const std::shared_ptr<Log> &log,
                     const std::shared_ptr<VarCollection> &varCollection,
                     const std::shared_ptr<EventQueue> &eventQueue,
                     const std::shared_ptr<SurfaceManager> &surfaceManager,
                     const std::shared_ptr<PhysicalDeviceProxy> &physicalDevice,
                     const std::shared_ptr<LogicalDeviceProxy> &logicalDevice);

    void init();
    void destroy();

    [[nodiscard]] std::shared_ptr<Swapchain> getSwapchainFor(const std::shared_ptr<Window> &window);
};

#endif // RENDERING_SWAPCHAINMANAGER_HPP
