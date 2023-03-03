#include "CommandExecutor.hpp"

#include <utility>

#include "PhysicalDevice.hpp"
#include "RenderingDevice.hpp"
#include "src/Rendering/Objects/FenceObject.hpp"
#include "src/Rendering/Objects/SemaphoreObject.hpp"

CommandExecution::CommandExecution(Command command,
                                   RenderingDevice *renderingDevice,
                                   VkCommandPool commandPool,
                                   VkCommandBuffer commandBuffer,
                                   bool oneTimeBuffer)
        : _command(std::move(command)),
          _renderingDevice(renderingDevice),
          _commandPool(commandPool),
          _commandBuffer(commandBuffer),
          _oneTimeBuffer(oneTimeBuffer) {
    //
}

CommandExecution::~CommandExecution() {
    if (!this->_oneTimeBuffer) {
        return;
    }

    this->_renderingDevice->freeCommandBuffers(this->_commandPool, 1, &this->_commandBuffer);
}

CommandExecution &CommandExecution::withFence(FenceObject *fence) {
    this->_fence = fence;

    return *this;
}

CommandExecution &CommandExecution::withWaitSemaphore(SemaphoreObject *semaphore) {
    this->_waitSemaphores.push_back(semaphore->getHandle());

    return *this;
}

CommandExecution &CommandExecution::withSignalSemaphore(SemaphoreObject *semaphore) {
    this->_signalSemaphores.push_back(semaphore->getHandle());

    return *this;
}

CommandExecution &CommandExecution::withWaitDstStageMask(VkPipelineStageFlags waitDstStageMask) {
    this->_waitDstStageMask = waitDstStageMask;

    return *this;
}

void CommandExecution::submit(bool waitQueueIdle) {
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
            .waitSemaphoreCount = static_cast<uint32_t>(this->_waitSemaphores.size()),
            .pWaitSemaphores = this->_waitSemaphores.data(),
            .pWaitDstStageMask = this->_waitDstStageMask.has_value()
                                 ? &this->_waitDstStageMask.value()
                                 : nullptr,
            .commandBufferCount = 1,
            .pCommandBuffers = &this->_commandBuffer,
            .signalSemaphoreCount = static_cast<uint32_t>(this->_signalSemaphores.size()),
            .pSignalSemaphores = this->_signalSemaphores.data()
    };

    VkFence fence = this->_fence != nullptr
                    ? this->_fence->getHandle()
                    : VK_NULL_HANDLE;

    vkEnsure(vkQueueSubmit(this->_renderingDevice->getGraphicsQueue(), 1, &submitInfo, fence));

    if (!waitQueueIdle) {
        return;
    }

    vkEnsure(vkQueueWaitIdle(this->_renderingDevice->getGraphicsQueue()));
}

CommandExecutor::CommandExecutor(RenderingDevice *renderingDevice)
        : _renderingDevice(renderingDevice) {
    uint32_t queueFamilyIdx = this->_renderingDevice->getPhysicalDevice()->getGraphicsQueueFamilyIdx();
    this->_commandPool = this->_renderingDevice->createCommandPool(queueFamilyIdx);

    std::vector<VkCommandBuffer> buffers = this->_renderingDevice->allocateCommandBuffers(
            this->_commandPool, this->_mainBuffers.size());
    std::copy(buffers.begin(), buffers.end(), this->_mainBuffers.begin());
}

CommandExecutor::~CommandExecutor() {
    this->_renderingDevice->freeCommandBuffers(this->_commandPool, this->_mainBuffers.size(),
                                               this->_mainBuffers.data());
    this->_renderingDevice->destroyCommandPool(this->_commandPool);
}

CommandExecution CommandExecutor::beginMainExecution(uint32_t frameIdx, Command command) {
    VkCommandBuffer commandBuffer = this->_mainBuffers[frameIdx];

    vkEnsure(vkResetCommandBuffer(commandBuffer, 0));

    return {
            std::move(command), this->_renderingDevice, this->_commandPool, commandBuffer, false
    };
}

CommandExecution CommandExecutor::beginOneTimeExecution(Command command) {
    std::vector<VkCommandBuffer> buffers = this->_renderingDevice->allocateCommandBuffers(this->_commandPool, 1);
    return {
            std::move(command), this->_renderingDevice, this->_commandPool, buffers[0], true
    };
}
