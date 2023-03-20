#include "ShaderObject.hpp"

#include "src/Rendering/VulkanObjectsAllocator.hpp"

ShaderObject::ShaderObject(const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator,
                           VkShaderModule shader)
        : _vulkanObjectsAllocator(vulkanObjectsAllocator),
          _shader(shader) {
    //
}

void ShaderObject::destroy() {
    this->_vulkanObjectsAllocator->destroyShader(this->_shader);
}

std::shared_ptr<ShaderObject> ShaderObject::create(
        const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator,
        const std::vector<char> &content) {
    VkShaderModule shader = vulkanObjectsAllocator->createShader(content);

    return std::make_shared<ShaderObject>(vulkanObjectsAllocator, shader);
}
