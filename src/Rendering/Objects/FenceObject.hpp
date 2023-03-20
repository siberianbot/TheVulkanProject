#ifndef RENDERING_OBJECTS_FENCEOBJECT_HPP
#define RENDERING_OBJECTS_FENCEOBJECT_HPP

#include <memory>

#include <vulkan/vulkan.hpp>

class RenderingDevice;

class FenceObject {
private:
    RenderingDevice *_renderingDevice;

    VkFence _fence;

public:
    FenceObject(RenderingDevice *renderingDevice, VkFence fence);
    ~FenceObject();

    [[nodiscard]] VkFence getHandle() const { return this->_fence; }

    void wait(uint64_t timeout);
    void reset();

    [[nodiscard]] static std::shared_ptr<FenceObject> create(RenderingDevice *renderingDevice, bool signaled);
};

#endif // RENDERING_OBJECTS_FENCEOBJECT_HPP
