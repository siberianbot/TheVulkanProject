#ifndef RENDERING_RENDERINGOBJECTSFACTORY_HPP
#define RENDERING_RENDERINGOBJECTSFACTORY_HPP

#include "Rendering/RenderingDevice.hpp"
#include "Rendering/BufferObject.hpp"
#include "Rendering/ImageObject.hpp"

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
};

#endif // RENDERING_RENDERINGOBJECTSFACTORY_HPP
