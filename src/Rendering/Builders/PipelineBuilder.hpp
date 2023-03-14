#ifndef RENDERING_BUILDERS_PIPELINEBUILDER_HPP
#define RENDERING_BUILDERS_PIPELINEBUILDER_HPP

#include <functional>
#include <optional>
#include <vector>

#include <vulkan/vulkan.hpp>

class RenderingDevice;
class SpecializationInfoBuilder;
class ShaderObject;

using ShaderSpecializationFunc = std::function<void(SpecializationInfoBuilder &)>;

class PipelineBuilder {
private:
    RenderingDevice *_renderingDevice;
    VkRenderPass _renderpass;
    VkPipelineLayout _pipelineLayout;

    ShaderObject *_vertexShader = nullptr;
    ShaderObject *_fragmentShader = nullptr;
    std::vector<VkVertexInputBindingDescription> _bindings;
    std::vector<VkVertexInputAttributeDescription> _attributes;
    std::optional<VkSpecializationInfo> _vertexShaderSpecialization;
    std::optional<VkSpecializationInfo> _fragmentShaderSpecialization;
    std::optional<VkCullModeFlags> _cullMode;
    std::optional<VkSampleCountFlagBits> _rasterizationSamples;
    std::optional<uint32_t> _colorBlendAttachmentCount;
    bool _depthBiasEnabled = false;
    uint32_t _subpassIdx = 0;

public:
    PipelineBuilder(RenderingDevice *renderingDevice, VkRenderPass renderpass, VkPipelineLayout pipelineLayout);

    PipelineBuilder &addVertexShader(ShaderObject *shader);
    PipelineBuilder &addFragmentShader(ShaderObject *shader);

    PipelineBuilder &addBinding(uint32_t bindingIdx, uint32_t stride, VkVertexInputRate inputRate);
    PipelineBuilder &addAttribute(uint32_t bindingIdx, uint32_t locationIdx,
                                  uint32_t offset, VkFormat format);

    PipelineBuilder &withVertexShaderSpecialization(ShaderSpecializationFunc func);
    PipelineBuilder &withFragmentShaderSpecialization(ShaderSpecializationFunc func);

    PipelineBuilder &withDepthBias();
    PipelineBuilder &withCullMode(VkCullModeFlags cullMode);
    PipelineBuilder &withRasterizationSamples(VkSampleCountFlagBits samples);
    PipelineBuilder &withColorBlendAttachmentCount(uint32_t count);

    PipelineBuilder &forSubpass(uint32_t subpass);

    VkPipeline build();
};

#endif // RENDERING_BUILDERS_PIPELINEBUILDER_HPP
