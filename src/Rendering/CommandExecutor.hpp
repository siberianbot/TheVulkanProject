#ifndef RENDERING_COMMANDEXECUTOR_HPP
#define RENDERING_COMMANDEXECUTOR_HPP

#include <memory>
#include <optional>

#include "src/Rendering/Constants.hpp"
#include "src/Rendering/Types/Command.hpp"

class PhysicalDevice;
class RenderingDevice;
class CommandExecution;
class RenderingFunctionsProxy;

class CommandExecutor {
private:
    std::shared_ptr<RenderingDevice> _renderingDevice;
    std::shared_ptr<RenderingFunctionsProxy> _renderingFunctions;

    VkCommandPool _commandPool = VK_NULL_HANDLE;
    std::array<VkCommandBuffer, MAX_INFLIGHT_FRAMES> _inflightBuffers;

public:
    CommandExecutor(const std::shared_ptr<RenderingDevice> &renderingDevice,
                    const std::shared_ptr<RenderingFunctionsProxy> &renderingFunctions);

    void init();
    void destroy();

    CommandExecution beginMainExecution(uint32_t frameIdx, Command command);
    CommandExecution beginOneTimeExecution(Command command);

    static std::shared_ptr<CommandExecutor> create(const std::shared_ptr<RenderingDevice> &renderingDevice,
                                                   const std::shared_ptr<RenderingFunctionsProxy> &renderingFunctions);
};

#endif // RENDERING_COMMANDEXECUTOR_HPP
