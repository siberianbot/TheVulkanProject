#ifndef RENDERING_COMMANDEXECUTION_HPP
#define RENDERING_COMMANDEXECUTION_HPP

#include <memory>
#include <optional>

#include "src/Rendering/Types/Command.hpp"

class RenderingDevice;
class FenceObject;
class SemaphoreObject;

class CommandExecution {
private:
    std::shared_ptr<RenderingDevice> _renderingDevice;
    Command _command;
    VkCommandPool _commandPool;
    VkCommandBuffer _commandBuffer;
    bool _isOneTimeBuffer;

    VkFence _fence = VK_NULL_HANDLE;
    std::vector<VkSemaphore> _waitSemaphores;
    std::vector<VkSemaphore> _signalSemaphores;
    std::vector<VkPipelineStageFlags> _waitDstStageMasks;

public:
    CommandExecution(const std::shared_ptr<RenderingDevice> &renderingDevice,
                     const Command &command,
                     VkCommandPool commandPool,
                     VkCommandBuffer commandBuffer,
                     bool isOneTimeBuffer);
    ~CommandExecution();

    CommandExecution &withFence(const std::shared_ptr<FenceObject> &fence);
    CommandExecution &withWaitSemaphore(const std::shared_ptr<SemaphoreObject> &semaphore);
    CommandExecution &withSignalSemaphore(const std::shared_ptr<SemaphoreObject> &semaphore);
    CommandExecution &withWaitDstStageMask(VkPipelineStageFlags waitDstStageMask);

    void submit(bool waitQueueIdle);
};


#endif // RENDERING_COMMANDEXECUTION_HPP
