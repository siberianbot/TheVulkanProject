#include "FenceObject.hpp"

#include "Rendering/RenderingDevice.hpp"

FenceObject::FenceObject(RenderingDevice *renderingDevice, VkFence fence)
        : _renderingDevice(renderingDevice),
          _fence(fence) {
    //
}

FenceObject::~FenceObject() {
    this->_renderingDevice->destroyFence(this->_fence);
}

void FenceObject::wait(uint64_t timeout) {
    this->_renderingDevice->waitForFence(this->_fence, timeout);
}

void FenceObject::reset() {
    this->_renderingDevice->resetFence(this->_fence);
}
