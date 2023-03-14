#ifndef RENDERING_OBJECTS_SHADEROBJECT_HPP
#define RENDERING_OBJECTS_SHADEROBJECT_HPP

#include <vulkan/vulkan.hpp>

class RenderingDevice;

class ShaderObject {
private:
    RenderingDevice *_renderingDevice;

    VkShaderModule _shader;

    ShaderObject(RenderingDevice *renderingDevice, VkShaderModule shader);

public:
    ~ShaderObject();

    [[nodiscard]] VkShaderModule getHandle() const { return this->_shader; }

    [[nodiscard]] static ShaderObject *create(RenderingDevice *renderingDevice, const std::vector<char> &content);
};

#endif // RENDERING_OBJECTS_SHADEROBJECT_HPP
