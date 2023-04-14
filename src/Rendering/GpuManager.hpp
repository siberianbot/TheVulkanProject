#ifndef RENDERING_GPUMANAGER_HPP
#define RENDERING_GPUMANAGER_HPP

#include <memory>
#include <vector>

#include <vulkan/vulkan.hpp>

class Log;
class VarCollection;
class EventQueue;
class ResourceDatabase;
class ResourceLoader;
class Window;

class CommandManager;
class GpuAllocator;
class GpuResourceManager;
class LogicalDeviceProxy;
class PhysicalDeviceProxy;

class GpuManager {
private:
    std::shared_ptr<Log> _log;
    std::shared_ptr<VarCollection> _varCollection;
    std::shared_ptr<EventQueue> _eventQueue;
    std::shared_ptr<ResourceDatabase> _resourceDatabase;
    std::shared_ptr<ResourceLoader> _resourceLoader;
    std::shared_ptr<Window> _window;

    vk::Instance _instance;
    vk::SurfaceKHR _surface;
    std::shared_ptr<PhysicalDeviceProxy> _physicalDevice;
    std::shared_ptr<LogicalDeviceProxy> _logicalDevice;
    std::shared_ptr<CommandManager> _commandManager;
    std::shared_ptr<GpuAllocator> _allocator;
    std::shared_ptr<GpuResourceManager> _resourceManager;

    std::vector<const char *> getRequiredInstanceExtensions();
    vk::PhysicalDeviceFeatures getEnabledFeatures();

    void initInstance();
    void initSurface();
    void initPhysicalDevice();
    void initLogicalDevice();
    void initCommandManager();
    void initAllocator();
    void initResourceManager();

    static VkBool32 messengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                      VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                      const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                      void *pUserData);

public:
    GpuManager(const std::shared_ptr<Log> &log,
               const std::shared_ptr<VarCollection> &varCollection,
               const std::shared_ptr<EventQueue> &eventQueue,
               const std::shared_ptr<ResourceDatabase> resourceDatabase,
               const std::shared_ptr<ResourceLoader> resourceLoader,
               const std::shared_ptr<Window> &window);

    void init();
    void destroy();

    [[nodiscard]] std::weak_ptr<PhysicalDeviceProxy> getPhysicalDeviceProxy() const { return this->_physicalDevice; }

    [[nodiscard]] std::weak_ptr<LogicalDeviceProxy> getLogicalDeviceProxy() const { return this->_logicalDevice; }

    [[nodiscard]] std::weak_ptr<CommandManager> getCommandManager() const { return this->_commandManager; }

    [[nodiscard]] std::weak_ptr<GpuAllocator> getAllocator() const { return this->_allocator; }

    [[nodiscard]] std::weak_ptr<GpuResourceManager> getResourceManager() const { return this->_resourceManager; }
};

#endif // RENDERING_GPUMANAGER_HPP
