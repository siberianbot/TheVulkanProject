#include "CommandManager.hpp"

#include "src/Engine/EngineError.hpp"
#include "src/Engine/Log.hpp"
#include "src/Rendering/Proxies/CommandBufferProxy.hpp"
#include "src/Rendering/Proxies/LogicalDeviceProxy.hpp"
#include "src/Rendering/Proxies/PhysicalDeviceProxy.hpp"

static constexpr const char *COMMAND_MANAGER_TAG = "CommandManager";

CommandManager::CommandManager(const std::shared_ptr<Log> &log,
                               const std::shared_ptr<PhysicalDeviceProxy> &physicalDevice,
                               const std::shared_ptr<LogicalDeviceProxy> &logicalDevice)
        : _log(log),
          _physicalDevice(physicalDevice),
          _logicalDevice(logicalDevice) {
    //
}

void CommandManager::init() {
    vk::CommandPoolCreateInfo createInfo = vk::CommandPoolCreateInfo()
            .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
            .setQueueFamilyIndex(this->_physicalDevice->getGraphicsQueueFamilyIdx());

    try {
        this->_commandPool = this->_logicalDevice->getHandle().createCommandPool(createInfo);
    } catch (const std::exception &error) {
        this->_log->error(COMMAND_MANAGER_TAG, error);
        throw EngineError("Failed to initialize command manager");
    }
}

void CommandManager::destroy() {
    this->_logicalDevice->getHandle().destroy(this->_commandPool);
}

std::shared_ptr<CommandBufferProxy> CommandManager::createPrimaryBuffer() const {
    vk::CommandBufferAllocateInfo allocateInfo = vk::CommandBufferAllocateInfo()
            .setCommandPool(this->_commandPool)
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(1);


    std::vector<vk::CommandBuffer> buffers;

    try {
        buffers = this->_logicalDevice->getHandle().allocateCommandBuffers(allocateInfo);
    } catch (const std::exception &error) {
        this->_log->error(COMMAND_MANAGER_TAG, error);
        throw EngineError("Failed to create primary command buffer");
    }

    return std::make_shared<CommandBufferProxy>(this->_logicalDevice, this->_commandPool, buffers[0]);
}
