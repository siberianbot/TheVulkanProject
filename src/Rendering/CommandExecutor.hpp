#ifndef RENDERING_COMMANDEXECUTOR_HPP
#define RENDERING_COMMANDEXECUTOR_HPP

#include <functional>
#include <optional>

#include "Common.hpp"

using Command = std::function<void(VkCommandBuffer cmdBuffer)>;

class RenderingDevice;
class FenceObject;
class SemaphoreObject;

class CommandExecution {
private:
    Command _command;
    VkDevice _device;
    VkCommandPool _commandPool;
    VkCommandBuffer _commandBuffer;
    VkQueue _queue;
    bool _oneTimeBuffer;

    FenceObject *_fence = nullptr;
    std::vector<VkSemaphore> _waitSemaphores;
    std::vector<VkSemaphore> _signalSemaphores;
    std::optional<VkPipelineStageFlags> _waitDstStageMask;

public:
    CommandExecution(Command command,
                     VkDevice device,
                     VkCommandPool commandPool,
                     VkCommandBuffer commandBuffer,
                     VkQueue queue,
                     bool oneTimeBuffer);
    ~CommandExecution();

    CommandExecution &withFence(FenceObject *fence);
    CommandExecution &withWaitSemaphore(SemaphoreObject *semaphore);
    CommandExecution &withSignalSemaphore(SemaphoreObject *semaphore);
    CommandExecution &withWaitDstStageMask(VkPipelineStageFlags waitDstStageMask);

    void submit(bool waitQueueIdle);
};

class CommandExecutor {
private:
    RenderingDevice *_renderingDevice;
    VkCommandPool _commandPool;
    std::array<VkCommandBuffer, MAX_INFLIGHT_FRAMES> _mainBuffers;

public:
    explicit CommandExecutor(RenderingDevice *renderingDevice);
    ~CommandExecutor();

    CommandExecution beginMainExecution(uint32_t frameIdx, Command command);
    CommandExecution beginOneTimeExecution(Command command);
};

#endif // RENDERING_COMMANDEXECUTOR_HPP
