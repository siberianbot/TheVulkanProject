#include "ShaderObject.hpp"

#include "src/Rendering/RenderingDevice.hpp"

ShaderObject::ShaderObject(RenderingDevice *renderingDevice, VkShaderModule shader)
        : _renderingDevice(renderingDevice),
          _shader(shader) {
    //
}

ShaderObject::~ShaderObject() {
    this->_renderingDevice->destroyShaderModule(this->_shader);
}

ShaderObject *ShaderObject::create(RenderingDevice *renderingDevice, const std::vector<char> &content) {
    VkShaderModule shader = renderingDevice->createShaderModule(content);

    return new ShaderObject(renderingDevice, shader);
}
