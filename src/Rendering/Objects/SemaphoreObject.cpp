#include "SemaphoreObject.hpp"

#include "src/Rendering/RenderingDevice.hpp"

SemaphoreObject::SemaphoreObject(RenderingDevice *renderingDevice, VkSemaphore semaphore)
        : _renderingDevice(renderingDevice),
          _semaphore(semaphore) {
    //
}

SemaphoreObject::~SemaphoreObject() {
    this->_renderingDevice->destroySemaphore(this->_semaphore);
}
