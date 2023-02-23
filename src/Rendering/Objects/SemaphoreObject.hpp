#ifndef RENDERING_OBJECTS_SEMAPHOREOBJECT_HPP
#define RENDERING_OBJECTS_SEMAPHOREOBJECT_HPP

#include <vulkan/vulkan.hpp>

class RenderingDevice;

class SemaphoreObject {
private:
    RenderingDevice *_renderingDevice;
    VkSemaphore _semaphore;

public:
    SemaphoreObject(RenderingDevice *renderingDevice, VkSemaphore semaphore);
    ~SemaphoreObject();

    [[nodiscard]] VkSemaphore getHandle() const { return this->_semaphore; }
};

#endif // RENDERING_OBJECTS_SEMAPHOREOBJECT_HPP
