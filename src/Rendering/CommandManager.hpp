#ifndef RENDERING_COMMANDMANAGER_HPP
#define RENDERING_COMMANDMANAGER_HPP

#include <memory>

#include <vulkan/vulkan.hpp>

class Log;
class CommandBufferProxy;
class LogicalDeviceProxy;
class PhysicalDeviceProxy;

class CommandManager {
private:
    std::shared_ptr<Log> _log;
    std::shared_ptr<PhysicalDeviceProxy> _physicalDevice;
    std::shared_ptr<LogicalDeviceProxy> _logicalDevice;

    vk::CommandPool _commandPool;

public:
    CommandManager(const std::shared_ptr<Log> &log,
                   const std::shared_ptr<PhysicalDeviceProxy> &physicalDevice,
                   const std::shared_ptr<LogicalDeviceProxy> &logicalDevice);

    void init();
    void destroy();

    [[nodiscard]] std::shared_ptr<CommandBufferProxy> createPrimaryBuffer() const;
};

#endif // RENDERING_COMMANDMANAGER_HPP
