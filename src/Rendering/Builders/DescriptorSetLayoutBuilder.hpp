#ifndef RENDERING_BUILDERS_DESCRIPTORSETLAYOUTBUILDER_HPP
#define RENDERING_BUILDERS_DESCRIPTORSETLAYOUTBUILDER_HPP

#include <memory>
#include <vector>

#include <vulkan/vulkan.hpp>

class VulkanObjectsAllocator;

class DescriptorSetLayoutBuilder {
private:
    std::shared_ptr<VulkanObjectsAllocator> _vulkanObjectsAllocator;

    std::vector<VkDescriptorSetLayoutBinding> _bindings;

public:
    DescriptorSetLayoutBuilder(const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator);

    DescriptorSetLayoutBuilder &withBinding(uint32_t idx, VkDescriptorType type, VkShaderStageFlags stages,
                                            uint32_t count = 1);

    VkDescriptorSetLayout build();
};

#endif // RENDERING_BUILDERS_DESCRIPTORSETLAYOUTBUILDER_HPP
