#include "DescriptorPoolBuilder.hpp"

#include "src/Rendering/VulkanObjectsAllocator.hpp"

DescriptorPoolBuilder::DescriptorPoolBuilder(const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator)
        : _vulkanObjectsAllocator(vulkanObjectsAllocator) {
    //
}

DescriptorPoolBuilder &DescriptorPoolBuilder::withSetCount(uint32_t count) {
    this->_setCount = count;

    return *this;
}

DescriptorPoolBuilder &DescriptorPoolBuilder::forType(VkDescriptorType type, uint32_t count) {
    this->_sizes.push_back(VkDescriptorPoolSize{
            .type = type,
            .descriptorCount = count
    });

    return *this;
}

VkDescriptorPool DescriptorPoolBuilder::build() {
    VkDescriptorPoolCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
            .maxSets = this->_setCount.value_or(DEFAULT_SET_COUNT),
            .poolSizeCount = static_cast<uint32_t>(this->_sizes.size()),
            .pPoolSizes = this->_sizes.data()
    };

    return this->_vulkanObjectsAllocator->createDescriptorPool(&createInfo);
}
