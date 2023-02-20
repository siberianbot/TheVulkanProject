#ifndef RENDERING_VULKANCOMMANDEXECUTOR_HPP
#define RENDERING_VULKANCOMMANDEXECUTOR_HPP

#include <functional>
#include <optional>

#include "VulkanConstants.hpp"

using VulkanCommand = std::function<void(VkCommandBuffer cmdBuffer)>;

class RenderingDevice;
class FenceObject;
class SemaphoreObject;

// TODO naming - remove Vulkan prefix
class VulkanCommandExecution {
private:
    VulkanCommand _command;
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
    VulkanCommandExecution(VulkanCommand command,
                           VkDevice device,
                           VkCommandPool commandPool,
                           VkCommandBuffer commandBuffer,
                           VkQueue queue,
                           bool oneTimeBuffer);
    ~VulkanCommandExecution();

    VulkanCommandExecution &withFence(FenceObject *fence);
    VulkanCommandExecution &withWaitSemaphore(SemaphoreObject *semaphore);
    VulkanCommandExecution &withSignalSemaphore(SemaphoreObject *semaphore);
    VulkanCommandExecution &withWaitDstStageMask(VkPipelineStageFlags waitDstStageMask);

    void submit(bool waitQueueIdle);
};

class VulkanCommandExecutor {
private:
    RenderingDevice *_renderingDevice;
    VkCommandPool _commandPool;
    std::array<VkCommandBuffer, MAX_INFLIGHT_FRAMES> _mainBuffers;

public:
    explicit VulkanCommandExecutor(RenderingDevice *renderingDevice);
    ~VulkanCommandExecutor();

    VulkanCommandExecution beginMainExecution(uint32_t frameIdx, VulkanCommand command);
    VulkanCommandExecution beginOneTimeExecution(VulkanCommand command);
};

#endif // RENDERING_VULKANCOMMANDEXECUTOR_HPP
