#ifndef RENDERING_RENDERINGOBJECTSFACTORY_HPP
#define RENDERING_RENDERINGOBJECTSFACTORY_HPP

#include <vulkan/vulkan.hpp>

class RenderingDevice;
class BufferObject;
class ImageObject;
class FenceObject;
class SemaphoreObject;

class RenderingObjectsFactory {
private:
    RenderingDevice *_renderingDevice;

public:
    explicit RenderingObjectsFactory(RenderingDevice *renderingDevice);

    BufferObject *createBufferObject(VkDeviceSize size, VkBufferUsageFlags usage,
                                     VkMemoryPropertyFlags memoryProperty);

    ImageObject *createImageObject(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage,
                                   VkSampleCountFlagBits samples, VkMemoryPropertyFlags memoryProperty,
                                   VkImageAspectFlags aspectMask);

    FenceObject *createFenceObject(bool signaled);

    SemaphoreObject *createSemaphoreObject();
};

#endif // RENDERING_RENDERINGOBJECTSFACTORY_HPP
