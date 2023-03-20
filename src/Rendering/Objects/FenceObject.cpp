#include "FenceObject.hpp"

#include "src/Rendering/Common.hpp"
#include "src/Rendering/RenderingDevice.hpp"
#include "src/Rendering/VulkanObjectsAllocator.hpp"

FenceObject::FenceObject(const std::shared_ptr<RenderingDevice> &renderingDevice,
                         const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator,
                         VkFence fence)
        : _renderingDevice(renderingDevice),
          _vulkanObjectsAllocator(vulkanObjectsAllocator),
          _fence(fence) {
    //
}

void FenceObject::destroy() {
    this->_vulkanObjectsAllocator->destroyFence(this->_fence);
}

void FenceObject::wait(uint64_t timeout) {
    vkEnsure(vkWaitForFences(this->_renderingDevice->getHandle(), 1, &this->_fence, VK_TRUE, timeout));
}

void FenceObject::reset() {
    vkEnsure(vkResetFences(this->_renderingDevice->getHandle(), 1, &this->_fence));
}

std::shared_ptr<FenceObject> FenceObject::create(const std::shared_ptr<RenderingDevice> &renderingDevice,
                                                 const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator,
                                                 bool signaled) {
    VkFence fence = vulkanObjectsAllocator->createFence(signaled);

    return std::make_shared<FenceObject>(renderingDevice, vulkanObjectsAllocator, fence);
}
