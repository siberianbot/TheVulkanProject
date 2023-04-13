#include "CommandExecutor.hpp"

#include "src/Rendering/CommandExecution.hpp"
#include "src/Rendering/Proxies/LogicalDeviceProxy.hpp"

CommandExecutor::CommandExecutor(const std::shared_ptr<LogicalDeviceProxy> &logicalDevice,
                                 const vk::CommandPool &commandPool,
                                 const std::vector<vk::CommandBuffer> &mainBuffers)
        : _logicalDevice(logicalDevice),
          _commandPool(commandPool),
          _mainBuffers(mainBuffers) {
    //
}

void CommandExecutor::destroy() {
    this->_logicalDevice->getHandle().freeCommandBuffers(this->_commandPool, this->_mainBuffers);
    this->_logicalDevice->getHandle().destroy(this->_commandPool);
}

CommandExecution CommandExecutor::beginMainExecution(uint32_t frameIdx, Command command) {
    VkCommandBuffer commandBuffer = this->_inflightBuffers[frameIdx];

    this->_mainBuffers[frameIdx]

    vkEnsure(vkResetCommandBuffer(commandBuffer, 0));

    return CommandExecution(this->_renderingDevice, command, this->_commandPool, commandBuffer, false);
}

CommandExecution CommandExecutor::beginOneTimeExecution(Command command) {
    VkCommandBuffer commandBuffer;
    VkCommandBufferAllocateInfo allocateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = this->_commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
    };

    vkEnsure(vkAllocateCommandBuffers(this->_renderingDevice->getHandle(), &allocateInfo, &commandBuffer));

    return CommandExecution(this->_renderingDevice, command, this->_commandPool, commandBuffer, true);
}
