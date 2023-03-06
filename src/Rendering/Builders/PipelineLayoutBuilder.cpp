#include "PipelineLayoutBuilder.hpp"

#include "src/Rendering/RenderingDevice.hpp"

PipelineLayoutBuilder::PipelineLayoutBuilder(RenderingDevice *renderingDevice)
        : _renderingDevice(renderingDevice) {
    //
}

PipelineLayoutBuilder &PipelineLayoutBuilder::withDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) {
    this->_descriptorSetLayouts.push_back(descriptorSetLayout);

    return *this;
}

PipelineLayoutBuilder &PipelineLayoutBuilder::withPushConstant(VkShaderStageFlags stages, uint32_t offset,
                                                               uint32_t size) {
    this->_pushConstants.push_back(VkPushConstantRange{
            .stageFlags = stages,
            .offset = offset,
            .size = size
    });

    return *this;
}

VkPipelineLayout PipelineLayoutBuilder::build() {
    return this->_renderingDevice->createPipelineLayout(this->_descriptorSetLayouts, this->_pushConstants);
}
