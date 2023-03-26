#include "DescriptorSetLayoutBuilder.hpp"

#include "src/Rendering/VulkanObjectsAllocator.hpp"

DescriptorSetLayoutBuilder::DescriptorSetLayoutBuilder(
        const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator)
        : _vulkanObjectsAllocator(vulkanObjectsAllocator) {
    //
}

DescriptorSetLayoutBuilder &DescriptorSetLayoutBuilder::withBinding(uint32_t idx, VkDescriptorType type,
                                                                    VkShaderStageFlags stages, uint32_t count) {
    this->_bindings.push_back(VkDescriptorSetLayoutBinding{
            .binding = idx,
            .descriptorType = type,
            .descriptorCount = count,
            .stageFlags = stages,
            .pImmutableSamplers = nullptr
    });

    return *this;
}

VkDescriptorSetLayout DescriptorSetLayoutBuilder::build() {
    VkDescriptorSetLayoutCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .bindingCount = static_cast<uint32_t>(this->_bindings.size()),
            .pBindings = this->_bindings.data()
    };

    return this->_vulkanObjectsAllocator->createDescriptorSetLayout(&createInfo);
}
