#include "ImageObject.hpp"

#include "src/Rendering/RenderingDevice.hpp"

ImageObject::ImageObject(RenderingDevice *renderingDevice, VkImage image, uint32_t layers, VkImageCreateFlags flags,
                         VkFormat format, VkDeviceMemory memory)
        : _renderingDevice(renderingDevice),
          _image(image),
          _layers(layers),
          _flags(flags),
          _format(format),
          _memory(memory) {
    //
}

ImageObject::~ImageObject() {
    this->_renderingDevice->freeMemory(this->_memory);
    this->_renderingDevice->destroyImage(this->_image);
}
