#ifndef RENDERING_BUILDERS_PIPELINELAYOUTBUILDER_HPP
#define RENDERING_BUILDERS_PIPELINELAYOUTBUILDER_HPP

#include <vector>

#include <vulkan/vulkan.hpp>

class RenderingDevice;

class PipelineLayoutBuilder {
private:
    RenderingDevice *_renderingDevice;

    std::vector<VkDescriptorSetLayout> _descriptorSetLayouts;
    std::vector<VkPushConstantRange> _pushConstants;

public:
    explicit PipelineLayoutBuilder(RenderingDevice *renderingDevice);

    PipelineLayoutBuilder &withDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout);
    PipelineLayoutBuilder &withPushConstant(VkShaderStageFlags stages, uint32_t offset, uint32_t size);

    VkPipelineLayout build();
};

#endif // RENDERING_BUILDERS_PIPELINELAYOUTBUILDER_HPP
