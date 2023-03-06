#ifndef RENDERING_BUILDERS_DESCRIPTORSETLAYOUTBUILDER_HPP
#define RENDERING_BUILDERS_DESCRIPTORSETLAYOUTBUILDER_HPP

#include <vector>

#include <vulkan/vulkan.hpp>

class RenderingDevice;

class DescriptorSetLayoutBuilder {
private:
    RenderingDevice *_renderingDevice;
    std::vector<VkDescriptorSetLayoutBinding> _bindings;

public:
    explicit DescriptorSetLayoutBuilder(RenderingDevice *renderingDevice);

    DescriptorSetLayoutBuilder &withBinding(uint32_t idx, VkDescriptorType type, VkShaderStageFlags stages);

    VkDescriptorSetLayout build();
};

#endif // RENDERING_BUILDERS_DESCRIPTORSETLAYOUTBUILDER_HPP
