#include "ImageObject.hpp"

#include "Rendering/RenderingDevice.hpp"

ImageObject::ImageObject(RenderingDevice *renderingDevice, VkImage image, VkFormat format, VkDeviceMemory memory)
        : _renderingDevice(renderingDevice),
          _image(image),
          _format(format),
          _memory(memory) {
    //
}

ImageObject::~ImageObject() {
    this->_renderingDevice->freeMemory(this->_memory);
    this->_renderingDevice->destroyImage(this->_image);
}
