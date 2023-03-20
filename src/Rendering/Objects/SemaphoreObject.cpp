#include "SemaphoreObject.hpp"

#include "src/Rendering/VulkanObjectsAllocator.hpp"

SemaphoreObject::SemaphoreObject(const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator,
                                 VkSemaphore semaphore)
        : _vulkanObjectsAllocator(vulkanObjectsAllocator),
          _semaphore(semaphore) {
    //
}

void SemaphoreObject::destroy() {
    this->_vulkanObjectsAllocator->destroySemaphore(this->_semaphore);
}

std::shared_ptr<SemaphoreObject> SemaphoreObject::create(
        const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator) {
    VkSemaphore semaphore = vulkanObjectsAllocator->createSemaphore();

    return std::make_shared<SemaphoreObject>(vulkanObjectsAllocator, semaphore);
}
