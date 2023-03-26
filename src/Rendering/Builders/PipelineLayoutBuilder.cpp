#include "PipelineLayoutBuilder.hpp"

#include "src/Rendering/VulkanObjectsAllocator.hpp"

PipelineLayoutBuilder::PipelineLayoutBuilder(const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator)
        : _vulkanObjectsAllocator(vulkanObjectsAllocator) {
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
    VkPipelineLayoutCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .setLayoutCount = static_cast<uint32_t>(this->_descriptorSetLayouts.size()),
            .pSetLayouts = this->_descriptorSetLayouts.data(),
            .pushConstantRangeCount = static_cast<uint32_t>(this->_pushConstants.size()),
            .pPushConstantRanges = this->_pushConstants.data()
    };

    return this->_vulkanObjectsAllocator->createPipelineLayout(&createInfo);
}
