#ifndef RENDERING_COMMANDEXECUTOR_HPP
#define RENDERING_COMMANDEXECUTOR_HPP

#include <memory>
#include <vector>

#include "src/Rendering/Types/Command.hpp"

class LogicalDeviceProxy;
class CommandExecution;

class CommandExecutor {
private:
    std::shared_ptr<LogicalDeviceProxy> _logicalDevice;
    vk::CommandPool _commandPool;
    std::vector<vk::CommandBuffer> _mainBuffers;

public:
    CommandExecutor(const std::shared_ptr<LogicalDeviceProxy> &logicalDevice,
                    const vk::CommandPool &commandPool,
                    const std::vector<vk::CommandBuffer> &mainBuffers);

    void destroy();

    CommandExecution beginMainExecution(uint32_t frameIdx, Command command);
    CommandExecution beginOneTimeExecution(Command command);
};

#endif // RENDERING_COMMANDEXECUTOR_HPP
