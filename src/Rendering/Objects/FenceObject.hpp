#ifndef RENDERING_OBJECTS_FENCEOBJECT_HPP
#define RENDERING_OBJECTS_FENCEOBJECT_HPP

#include <vulkan/vulkan.hpp>

class RenderingDevice;

class FenceObject {
private:
    RenderingDevice *_renderingDevice;

    VkFence _fence;

    FenceObject(RenderingDevice *renderingDevice, VkFence fence);

public:
    ~FenceObject();

    [[nodiscard]] VkFence getHandle() const { return this->_fence; }

    void wait(uint64_t timeout);
    void reset();

    [[nodiscard]] static FenceObject *create(RenderingDevice *renderingDevice, bool signaled);
};

#endif // RENDERING_OBJECTS_FENCEOBJECT_HPP
