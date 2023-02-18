#include "VulkanCommandExecutor.hpp"

#include <utility>

#include "VulkanCommon.hpp"
#include "Rendering/VulkanPhysicalDevice.hpp"

VulkanCommandExecution::VulkanCommandExecution(VulkanCommand command,
                                               VkDevice device,
                                               VkCommandPool commandPool,
                                               VkCommandBuffer commandBuffer,
                                               VkQueue queue,
                                               bool oneTimeBuffer)
        : _command(std::move(command)),
          _device(device),
          _commandPool(commandPool),
          _commandBuffer(commandBuffer),
          _queue(queue),
          _oneTimeBuffer(oneTimeBuffer) {
    //
}

VulkanCommandExecution::~VulkanCommandExecution() {
    if (!this->_oneTimeBuffer) {
        return;
    }

    vkFreeCommandBuffers(this->_device, this->_commandPool, 1, &this->_commandBuffer);
}

VulkanCommandExecution &VulkanCommandExecution::withFence(VkFence fence) {
    this->_fence = fence;

    return *this;
}

VulkanCommandExecution &VulkanCommandExecution::withWaitSemaphore(VkSemaphore semaphore) {
    this->_waitSemaphore = semaphore;

    return *this;
}

VulkanCommandExecution &VulkanCommandExecution::withSignalSemaphore(VkSemaphore semaphore) {
    this->_signalSemaphore = semaphore;

    return *this;
}

VulkanCommandExecution &VulkanCommandExecution::withWaitDstStageMask(VkPipelineStageFlags waitDstStageMask) {
    this->_waitDstStageMask = waitDstStageMask;

    return *this;
}

void VulkanCommandExecution::submit(bool waitQueueIdle) {
    VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = this->_oneTimeBuffer
                     ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
                     : static_cast<VkCommandBufferUsageFlags>(0),
            .pInheritanceInfo = nullptr
    };

    vkEnsure(vkBeginCommandBuffer(this->_commandBuffer, &beginInfo));

    this->_command(this->_commandBuffer);

    vkEnsure(vkEndCommandBuffer(this->_commandBuffer));

    VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = static_cast<uint32_t>(this->_waitSemaphore != VK_NULL_HANDLE ? 1 : 0),
            .pWaitSemaphores = this->_waitSemaphore != VK_NULL_HANDLE
                               ? &this->_waitSemaphore
                               : nullptr,
            .pWaitDstStageMask = this->_waitDstStageMask.has_value()
                                 ? &this->_waitDstStageMask.value()
                                 : nullptr,
            .commandBufferCount = 1,
            .pCommandBuffers = &this->_commandBuffer,
            .signalSemaphoreCount = static_cast<uint32_t>(this->_signalSemaphore != VK_NULL_HANDLE ? 1 : 0),
            .pSignalSemaphores = this->_signalSemaphore != VK_NULL_HANDLE
                                 ? &this->_signalSemaphore
                                 : nullptr
    };

    VkFence fence = _fence != VK_NULL_HANDLE
                    ? _fence
                    : VK_NULL_HANDLE;

    vkEnsure(vkQueueSubmit(this->_queue, 1, &submitInfo, fence));

    if (!waitQueueIdle) {
        return;
    }

    vkEnsure(vkQueueWaitIdle(this->_queue));
}

VulkanCommandExecutor::VulkanCommandExecutor(RenderingDevice *renderingDevice)
        : _renderingDevice(renderingDevice) {
    VkCommandPoolCreateInfo commandPoolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = this->_renderingDevice->getPhysicalDevice()->getGraphicsQueueFamilyIdx()
    };

    vkEnsure(vkCreateCommandPool(this->_renderingDevice->getHandle(), &commandPoolCreateInfo, nullptr,
                                 &this->_commandPool));

    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = this->_commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = static_cast<uint32_t>(this->_mainBuffers.size())
    };

    vkEnsure(vkAllocateCommandBuffers(this->_renderingDevice->getHandle(), &commandBufferAllocateInfo,
                                      this->_mainBuffers.data()));
}

VulkanCommandExecutor::~VulkanCommandExecutor() {
    vkFreeCommandBuffers(this->_renderingDevice->getHandle(), this->_commandPool,
                         static_cast<uint32_t>(this->_mainBuffers.size()), this->_mainBuffers.data());
    vkDestroyCommandPool(this->_renderingDevice->getHandle(), this->_commandPool, nullptr);
}

VulkanCommandExecution VulkanCommandExecutor::beginMainExecution(uint32_t frameIdx, VulkanCommand command) {
    VkCommandBuffer commandBuffer = this->_mainBuffers[frameIdx];

    vkEnsure(vkResetCommandBuffer(commandBuffer, 0));

    return {
            std::move(command), this->_renderingDevice->getHandle(),
            this->_commandPool, commandBuffer, this->_renderingDevice->getGraphicsQueue(),
            false
    };
}

VulkanCommandExecution VulkanCommandExecutor::beginOneTimeExecution(VulkanCommand command) {
    VkCommandBuffer commandBuffer;

    VkCommandBufferAllocateInfo allocateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = this->_commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
    };
    vkEnsure(vkAllocateCommandBuffers(this->_renderingDevice->getHandle(), &allocateInfo, &commandBuffer));

    return {
            std::move(command), this->_renderingDevice->getHandle(),
            this->_commandPool, commandBuffer, this->_renderingDevice->getGraphicsQueue(),
            true
    };
}
