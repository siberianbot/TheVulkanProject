#include "CommandExecutor.hpp"

#include "src/Rendering/Common.hpp"
#include "src/Rendering/RenderingDevice.hpp"
#include "src/Rendering/VulkanObjectsAllocator.hpp"
#include "src/Rendering/CommandExecution.hpp"

CommandExecutor::CommandExecutor(const std::shared_ptr<RenderingDevice> &renderingDevice,
                                 const std::shared_ptr<VulkanObjectsAllocator> &renderingFunctions)
        : _renderingDevice(renderingDevice),
          _vulkanObjectsAllocator(renderingFunctions) {
    //
}

void CommandExecutor::init() {
    this->_commandPool = this->_vulkanObjectsAllocator->createCommandPool();

    VkCommandBufferAllocateInfo allocateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = this->_commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = static_cast<uint32_t>(this->_inflightBuffers.size())
    };

    vkEnsure(vkAllocateCommandBuffers(this->_renderingDevice->getHandle(), &allocateInfo,
                                      this->_inflightBuffers.data()));
}

void CommandExecutor::destroy() {
    vkFreeCommandBuffers(this->_renderingDevice->getHandle(), this->_commandPool, this->_inflightBuffers.size(),
                         this->_inflightBuffers.data());

    this->_vulkanObjectsAllocator->destroyCommandPool(this->_commandPool);
}

CommandExecution CommandExecutor::beginMainExecution(uint32_t frameIdx, Command command) {
    VkCommandBuffer commandBuffer = this->_inflightBuffers[frameIdx];

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
