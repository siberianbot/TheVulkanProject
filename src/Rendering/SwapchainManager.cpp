#include "SwapchainManager.hpp"

#include "src/Events/EventQueue.hpp"
#include "src/Rendering/Swapchain.hpp"

SwapchainManager::SwapchainManager(const std::shared_ptr<Log> &log,
                                   const std::shared_ptr<VarCollection> &varCollection,
                                   const std::shared_ptr<EventQueue> &eventQueue,
                                   const std::shared_ptr<SurfaceManager> &surfaceManager,
                                   const std::shared_ptr<PhysicalDeviceProxy> &physicalDevice,
                                   const std::shared_ptr<LogicalDeviceProxy> &logicalDevice)
        : _log(log),
          _varCollection(varCollection),
          _eventQueue(eventQueue),
          _surfaceManager(surfaceManager),
          _physicalDevice(physicalDevice),
          _logicalDevice(logicalDevice) {
    //
}

void SwapchainManager::init() {
    this->_handlerIdx = this->_eventQueue->addHandler([this](const Event &event) {
        if (event.type != RESIZE_WINDOW_EVENT) {
            return;
        }

        auto it = this->_swapchains.find(event.window());

        if (it == this->_swapchains.end()) {
            return;
        }

        it->second->destroy();
        it->second->create();
    });
}

void SwapchainManager::destroy() {
    for (const auto &[window, swapchain]: this->_swapchains) {
        swapchain->destroy();
    }

    this->_swapchains.clear();

    this->_eventQueue->removeHandler(this->_handlerIdx);
}

std::shared_ptr<Swapchain> SwapchainManager::getSwapchainFor(const std::shared_ptr<Window> &window) {
    auto it = this->_swapchains.find(window.get());

    if (it != this->_swapchains.end()) {
        return it->second;
    }

    auto swapchain = std::make_shared<Swapchain>(this->_log,
                                                 this->_varCollection,
                                                 this->_surfaceManager,
                                                 this->_physicalDevice,
                                                 this->_logicalDevice,
                                                 window);

    this->_swapchains[window.get()] = swapchain;

    return swapchain;
}
