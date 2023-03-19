#ifndef RENDERING_COMMANDEXECUTOR_HPP
#define RENDERING_COMMANDEXECUTOR_HPP

#include <memory>
#include <optional>

#include "src/Rendering/Types/Command.hpp"
#include "Constants.hpp"

class PhysicalDevice;
class RenderingDevice;
class CommandExecution;

class CommandExecutor {
private:
    std::shared_ptr<RenderingDevice> _renderingDevice;
    VkCommandPool _commandPool;
    std::array<VkCommandBuffer, MAX_INFLIGHT_FRAMES> _mainBuffers;

public:
    CommandExecutor(const std::shared_ptr<RenderingDevice> &renderingDevice,
                    VkCommandPool commandPool, const std::array<VkCommandBuffer, MAX_INFLIGHT_FRAMES> &mainBuffers);

    void destroy();

    CommandExecution beginMainExecution(uint32_t frameIdx, Command command);
    CommandExecution beginOneTimeExecution(Command command);

    static std::shared_ptr<CommandExecutor> create(const std::shared_ptr<PhysicalDevice> &physicalDevice,
                                                   const std::shared_ptr<RenderingDevice> &renderingDevice);
};

#endif // RENDERING_COMMANDEXECUTOR_HPP
