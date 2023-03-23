#ifndef RENDERING_COMMANDEXECUTOR_HPP
#define RENDERING_COMMANDEXECUTOR_HPP

#include <memory>
#include <optional>

#include "src/Rendering/Constants.hpp"
#include "src/Rendering/Types/Command.hpp"

class PhysicalDevice;
class RenderingDevice;
class CommandExecution;
class VulkanObjectsAllocator;

class CommandExecutor {
private:
    std::shared_ptr<RenderingDevice> _renderingDevice;
    std::shared_ptr<VulkanObjectsAllocator> _vulkanObjectsAllocator;

    VkCommandPool _commandPool = VK_NULL_HANDLE;
    std::array<VkCommandBuffer, MAX_INFLIGHT_FRAMES> _inflightBuffers;

public:
    CommandExecutor(const std::shared_ptr<RenderingDevice> &renderingDevice,
                    const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator);

    void init();
    void destroy();

    CommandExecution beginMainExecution(uint32_t frameIdx, Command command);
    CommandExecution beginOneTimeExecution(Command command);
};

#endif // RENDERING_COMMANDEXECUTOR_HPP
