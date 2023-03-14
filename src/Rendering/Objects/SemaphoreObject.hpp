#ifndef RENDERING_OBJECTS_SEMAPHOREOBJECT_HPP
#define RENDERING_OBJECTS_SEMAPHOREOBJECT_HPP

#include <vulkan/vulkan.hpp>

class RenderingDevice;

class SemaphoreObject {
private:
    RenderingDevice *_renderingDevice;

    VkSemaphore _semaphore;

    SemaphoreObject(RenderingDevice *renderingDevice, VkSemaphore semaphore);

public:
    ~SemaphoreObject();

    [[nodiscard]] VkSemaphore getHandle() const { return this->_semaphore; }

    [[nodiscard]] static SemaphoreObject *create(RenderingDevice *renderingDevice);
};

#endif // RENDERING_OBJECTS_SEMAPHOREOBJECT_HPP
