#ifndef RENDERING_OBJECTS_SEMAPHOREOBJECT_HPP
#define RENDERING_OBJECTS_SEMAPHOREOBJECT_HPP

#include <memory>

#include <vulkan/vulkan.hpp>

class VulkanObjectsAllocator;

class SemaphoreObject {
private:
    std::shared_ptr<VulkanObjectsAllocator> _vulkanObjectsAllocator;

    VkSemaphore _semaphore;

public:
    SemaphoreObject(const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator, VkSemaphore semaphore);

    [[nodiscard]] VkSemaphore getHandle() const { return this->_semaphore; }

    void destroy();

    [[nodiscard]] static std::shared_ptr<SemaphoreObject> create(
            const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator);
};

#endif // RENDERING_OBJECTS_SEMAPHOREOBJECT_HPP
