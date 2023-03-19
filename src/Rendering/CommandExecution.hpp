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

    FenceObject *_fence = nullptr;
    std::vector<VkSemaphore> _waitSemaphores;
    std::vector<VkSemaphore> _signalSemaphores;
    std::optional<VkPipelineStageFlags> _waitDstStageMask;

public:
    CommandExecution(const std::shared_ptr<RenderingDevice> &renderingDevice,
                     const Command &command,
                     VkCommandPool commandPool,
                     VkCommandBuffer commandBuffer,
                     bool isOneTimeBuffer);
    ~CommandExecution();

    CommandExecution &withFence(FenceObject *fence);
    CommandExecution &withWaitSemaphore(SemaphoreObject *semaphore);
    CommandExecution &withSignalSemaphore(SemaphoreObject *semaphore);
    CommandExecution &withWaitDstStageMask(VkPipelineStageFlags waitDstStageMask);

    void submit(bool waitQueueIdle);
};


#endif // RENDERING_COMMANDEXECUTION_HPP
