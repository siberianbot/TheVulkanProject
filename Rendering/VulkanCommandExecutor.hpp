#ifndef RENDERING_VULKANCOMMANDEXECUTOR_HPP
#define RENDERING_VULKANCOMMANDEXECUTOR_HPP

#include <functional>
#include <optional>

#include "AnotherVulkanTypes.hpp"
#include "VulkanConstants.hpp"

using VulkanCommand = std::function<void(VkCommandBuffer cmdBuffer)>;

class VulkanCommandExecution {
private:
    VulkanCommand _command;
    VkDevice _device;
    VkCommandPool _commandPool;
    VkCommandBuffer _commandBuffer;
    VkQueue _queue;
    bool _oneTimeBuffer;

    VkFence _fence = VK_NULL_HANDLE;
    VkSemaphore _waitSemaphore = VK_NULL_HANDLE;
    VkSemaphore _signalSemaphore = VK_NULL_HANDLE;
    std::optional<VkPipelineStageFlags> _waitDstStageMask;

public:
    VulkanCommandExecution(VulkanCommand command,
                           VkDevice device,
                           VkCommandPool commandPool,
                           VkCommandBuffer commandBuffer,
                           VkQueue queue,
                           bool oneTimeBuffer);
    ~VulkanCommandExecution();

    VulkanCommandExecution &withFence(VkFence fence);
    VulkanCommandExecution &withWaitSemaphore(VkSemaphore semaphore);
    VulkanCommandExecution &withSignalSemaphore(VkSemaphore semaphore);
    VulkanCommandExecution &withWaitDstStageMask(VkPipelineStageFlags waitDstStageMask);

    void submit(bool waitQueueIdle);
};

class VulkanCommandExecutor {
private:
    RenderingDevice _renderingDevice;
    VkCommandPool _commandPool;
    std::array<VkCommandBuffer, MAX_INFLIGHT_FRAMES> _mainBuffers;

public:
    explicit VulkanCommandExecutor(const RenderingDevice &renderingDevice);
    ~VulkanCommandExecutor();

    VulkanCommandExecution beginMainExecution(uint32_t frameIdx, VulkanCommand command);
    VulkanCommandExecution beginOneTimeExecution(VulkanCommand command);
};

#endif // RENDERING_VULKANCOMMANDEXECUTOR_HPP
