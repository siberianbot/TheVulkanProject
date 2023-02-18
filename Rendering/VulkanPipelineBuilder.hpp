#ifndef RENDERING_VULKANPIPELINEBUILDER_HPP
#define RENDERING_VULKANPIPELINEBUILDER_HPP

#include "Rendering/RenderingDevice.hpp"

// TODO naming - remove Vulkan prefix
class VulkanPipelineBuilder {
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
    VulkanPipelineBuilder(RenderingDevice *renderingDevice, VkRenderPass renderpass, VkPipelineLayout pipelineLayout);
    ~VulkanPipelineBuilder();

    VulkanPipelineBuilder &addVertexShader(const std::string &path);
    VulkanPipelineBuilder &addFragmentShader(const std::string &path);

    VulkanPipelineBuilder &addBinding(uint32_t bindingIdx, uint32_t stride, VkVertexInputRate inputRate);
    VulkanPipelineBuilder &addAttribute(uint32_t bindingIdx, uint32_t locationIdx,
                                        uint32_t offset, VkFormat format);

    VkPipeline build();
};

#endif // RENDERING_VULKANPIPELINEBUILDER_HPP
