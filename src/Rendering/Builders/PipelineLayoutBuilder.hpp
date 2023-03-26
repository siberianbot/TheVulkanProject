#ifndef RENDERING_BUILDERS_PIPELINELAYOUTBUILDER_HPP
#define RENDERING_BUILDERS_PIPELINELAYOUTBUILDER_HPP

#include <memory>
#include <vector>

#include <vulkan/vulkan.hpp>

class VulkanObjectsAllocator;

class PipelineLayoutBuilder {
private:
    std::shared_ptr<VulkanObjectsAllocator> _vulkanObjectsAllocator;

    std::vector<VkDescriptorSetLayout> _descriptorSetLayouts;
    std::vector<VkPushConstantRange> _pushConstants;

public:
    explicit PipelineLayoutBuilder(const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator);

    PipelineLayoutBuilder &withDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout);
    PipelineLayoutBuilder &withPushConstant(VkShaderStageFlags stages, uint32_t offset, uint32_t size);

    VkPipelineLayout build();
};

#endif // RENDERING_BUILDERS_PIPELINELAYOUTBUILDER_HPP
