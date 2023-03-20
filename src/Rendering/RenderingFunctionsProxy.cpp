#include "RenderingFunctionsProxy.hpp"

#include "src/Rendering/Common.hpp"
#include "src/Rendering/PhysicalDevice.hpp"
#include "src/Rendering/RenderingDevice.hpp"

RenderingFunctionsProxy::RenderingFunctionsProxy(const std::shared_ptr<PhysicalDevice> &physicalDevice,
                                                 const std::shared_ptr<RenderingDevice> &renderingDevice)
        : _physicalDevice(physicalDevice),
          _renderingDevice(renderingDevice) {
    //
}

std::shared_ptr<RenderingFunctionsProxy>
RenderingFunctionsProxy::create(const std::shared_ptr<PhysicalDevice> &physicalDevice,
                                const std::shared_ptr<RenderingDevice> &renderingDevice) {
    return std::make_shared<RenderingFunctionsProxy>(physicalDevice, renderingDevice);
}

VkCommandPool RenderingFunctionsProxy::createCommandPool() {
    VkCommandPoolCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = this->_physicalDevice->getGraphicsQueueFamilyIdx()
    };

    VkCommandPool commandPool;
    vkEnsure(vkCreateCommandPool(this->_renderingDevice->getHandle(), &createInfo, nullptr, &commandPool));

    return commandPool;
}

void RenderingFunctionsProxy::destroyCommandPool(VkCommandPool commandPool) {
    vkDestroyCommandPool(this->_renderingDevice->getHandle(), commandPool, nullptr);
}
