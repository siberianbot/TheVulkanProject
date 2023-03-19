#include "CommandExecutor.hpp"

#include "src/Rendering/Common.hpp"
#include "src/Rendering/CommandExecution.hpp"
#include "src/Rendering/PhysicalDevice.hpp"
#include "src/Rendering/RenderingDevice.hpp"

CommandExecutor::CommandExecutor(const std::shared_ptr<RenderingDevice> &renderingDevice, VkCommandPool commandPool,
                                 const std::array<VkCommandBuffer, MAX_INFLIGHT_FRAMES> &mainBuffers)
        : _renderingDevice(renderingDevice),
          _commandPool(commandPool),
          _mainBuffers(mainBuffers) {
    //
}

void CommandExecutor::destroy() {
    vkFreeCommandBuffers(this->_renderingDevice->getHandle(), this->_commandPool, this->_mainBuffers.size(),
                         this->_mainBuffers.data());

    vkDestroyCommandPool(this->_renderingDevice->getHandle(), this->_commandPool, nullptr);
}

CommandExecution CommandExecutor::beginMainExecution(uint32_t frameIdx, Command command) {
    VkCommandBuffer commandBuffer = this->_mainBuffers[frameIdx];

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

std::shared_ptr<CommandExecutor> CommandExecutor::create(const std::shared_ptr<PhysicalDevice> &physicalDevice,
                                                         const std::shared_ptr<RenderingDevice> &renderingDevice) {
    VkCommandPoolCreateInfo commandPoolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = physicalDevice->getGraphicsQueueFamilyIdx()
    };

    VkCommandPool commandPool;
    vkEnsure(vkCreateCommandPool(renderingDevice->getHandle(), &commandPoolCreateInfo, nullptr, &commandPool));

    std::array<VkCommandBuffer, MAX_INFLIGHT_FRAMES> mainBuffers;
    VkCommandBufferAllocateInfo allocateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = static_cast<uint32_t>(mainBuffers.size())
    };

    vkEnsure(vkAllocateCommandBuffers(renderingDevice->getHandle(), &allocateInfo, mainBuffers.data()));

    return std::make_shared<CommandExecutor>(renderingDevice, commandPool, mainBuffers);
}
