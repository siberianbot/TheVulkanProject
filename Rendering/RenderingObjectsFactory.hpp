#ifndef RENDERING_RENDERINGOBJECTSFACTORY_HPP
#define RENDERING_RENDERINGOBJECTSFACTORY_HPP

#include "Rendering/RenderingDevice.hpp"
#include "Rendering/Objects/BufferObject.hpp"
#include "Rendering/Objects/ImageObject.hpp"
#include "Rendering/Objects/FenceObject.hpp"
#include "Rendering/Objects/SemaphoreObject.hpp"

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
