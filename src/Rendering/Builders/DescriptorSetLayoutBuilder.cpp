#include "DescriptorSetLayoutBuilder.hpp"

#include "src/Rendering/RenderingDevice.hpp"

DescriptorSetLayoutBuilder::DescriptorSetLayoutBuilder(RenderingDevice *renderingDevice)
        : _renderingDevice(renderingDevice) {
    //
}

DescriptorSetLayoutBuilder &DescriptorSetLayoutBuilder::withBinding(uint32_t idx, VkDescriptorType type,
                                                                    VkShaderStageFlags stages) {
    this->_bindings.push_back(VkDescriptorSetLayoutBinding{
            .binding = idx,
            .descriptorType = type,
            .descriptorCount = 1,
            .stageFlags = stages,
            .pImmutableSamplers = nullptr
    });

    return *this;
}

VkDescriptorSetLayout DescriptorSetLayoutBuilder::build() {
    return this->_renderingDevice->createDescriptorSetLayout(this->_bindings);
}