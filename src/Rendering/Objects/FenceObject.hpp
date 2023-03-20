#ifndef RENDERING_OBJECTS_FENCEOBJECT_HPP
#define RENDERING_OBJECTS_FENCEOBJECT_HPP

#include <memory>

#include <vulkan/vulkan.hpp>

class RenderingDevice;
class VulkanObjectsAllocator;

class FenceObject {
private:
    std::shared_ptr<RenderingDevice> _renderingDevice;
    std::shared_ptr<VulkanObjectsAllocator> _vulkanObjectsAllocator;

    VkFence _fence;

public:
    FenceObject(const std::shared_ptr<RenderingDevice> &renderingDevice,
                const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator,
                VkFence fence);

    [[nodiscard]] VkFence getHandle() const { return this->_fence; }

    void destroy();

    void wait(uint64_t timeout);
    void reset();

    [[nodiscard]] static std::shared_ptr<FenceObject> create(const std::shared_ptr<RenderingDevice> &renderingDevice,
                                                             const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator,
                                                             bool signaled);
};

#endif // RENDERING_OBJECTS_FENCEOBJECT_HPP
