#ifndef RENDERING_OBJECTS_SHADEROBJECT_HPP
#define RENDERING_OBJECTS_SHADEROBJECT_HPP

#include <memory>

#include <vulkan/vulkan.hpp>

class VulkanObjectsAllocator;

class ShaderObject {
private:
    std::shared_ptr<VulkanObjectsAllocator> _vulkanObjectsAllocator;

    VkShaderModule _shader;

public:
    ShaderObject(const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator,
                 VkShaderModule shader);

    [[nodiscard]] VkShaderModule getHandle() const { return this->_shader; }

    void destroy();

    [[nodiscard]] static std::shared_ptr<ShaderObject> create(
            const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator,
            const std::vector<char> &content);
};

#endif // RENDERING_OBJECTS_SHADEROBJECT_HPP
