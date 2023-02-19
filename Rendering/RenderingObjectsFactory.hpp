#ifndef RENDERING_RENDERINGOBJECTSFACTORY_HPP
#define RENDERING_RENDERINGOBJECTSFACTORY_HPP

#include "Rendering/RenderingDevice.hpp"
#include "Rendering/BufferObject.hpp"

class RenderingObjectsFactory {
private:
    RenderingDevice *_renderingDevice;

public:
    explicit RenderingObjectsFactory(RenderingDevice *renderingDevice);

    BufferObject *createBufferObject(VkDeviceSize size, VkBufferUsageFlags usage,
                                     VkMemoryPropertyFlags memoryProperty);
};

#endif // RENDERING_RENDERINGOBJECTSFACTORY_HPP
