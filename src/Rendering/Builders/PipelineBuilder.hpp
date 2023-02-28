#ifndef RENDERING_BUILDERS_PIPELINEBUILDER_HPP
#define RENDERING_BUILDERS_PIPELINEBUILDER_HPP

#include <vulkan/vulkan.hpp>

class RenderingDevice;

class PipelineBuilder {
private:
    RenderingDevice *_renderingDevice;
    VkRenderPass _renderpass;
    VkPipelineLayout _pipelineLayout;

    VkShaderModule _vertexShader = VK_NULL_HANDLE;
    VkShaderModule _fragmentShader = VK_NULL_HANDLE;
    std::vector<VkVertexInputBindingDescription> _bindings;
    std::vector<VkVertexInputAttributeDescription> _attributes;

    VkShaderModule createShaderModule(const std::string &path);

public:
    PipelineBuilder(RenderingDevice *renderingDevice, VkRenderPass renderpass, VkPipelineLayout pipelineLayout);
    ~PipelineBuilder();

    PipelineBuilder &addVertexShader(const std::string &path);
    PipelineBuilder &addFragmentShader(const std::string &path);

    PipelineBuilder &addBinding(uint32_t bindingIdx, uint32_t stride, VkVertexInputRate inputRate);
    PipelineBuilder &addAttribute(uint32_t bindingIdx, uint32_t locationIdx,
                                  uint32_t offset, VkFormat format);

    VkPipeline build();
};

#endif // RENDERING_BUILDERS_PIPELINEBUILDER_HPP
