#include "CommandExecution.hpp"

#include "src/Rendering/Common.hpp"
#include "src/Rendering/RenderingDevice.hpp"
#include "src/Rendering/Objects/FenceObject.hpp"
#include "src/Rendering/Objects/SemaphoreObject.hpp"

CommandExecution::CommandExecution(const std::shared_ptr<RenderingDevice> &renderingDevice,
                                   const Command &command,
                                   VkCommandPool commandPool,
                                   VkCommandBuffer commandBuffer,
                                   bool isOneTimeBuffer)
        : _renderingDevice(renderingDevice),
          _command(command),
          _commandPool(commandPool),
          _commandBuffer(commandBuffer),
          _isOneTimeBuffer(isOneTimeBuffer) {
    //
}

CommandExecution::~CommandExecution() {
    if (!this->_isOneTimeBuffer) {
        return;
    }

    vkFreeCommandBuffers(this->_renderingDevice->getHandle(), this->_commandPool, 1, &this->_commandBuffer);
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
            .flags = this->_isOneTimeBuffer
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
