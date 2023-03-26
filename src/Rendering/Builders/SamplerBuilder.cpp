#include "SamplerBuilder.hpp"

#include "src/Rendering/VulkanObjectsAllocator.hpp"

SamplerBuilder::SamplerBuilder(const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator)
        : _vulkanObjectsAllocator(vulkanObjectsAllocator) {
    //
}

SamplerBuilder &SamplerBuilder::enableAnisotropy(float maxAnisotropy) {
    this->_maxAnisotropy = maxAnisotropy;

    return *this;
}

SamplerBuilder &SamplerBuilder::enableComparison(VkCompareOp compareOp) {
    this->_compareOp = compareOp;

    return *this;
}

SamplerBuilder &SamplerBuilder::withAddressMode(VkSamplerAddressMode addressMode) {
    this->_addressMode = addressMode;

    return *this;
}

SamplerBuilder &SamplerBuilder::withBorderColor(VkBorderColor borderColor) {
    this->_borderColor = borderColor;

    return *this;
}

VkSampler SamplerBuilder::build() {
    VkSamplerAddressMode addressMode = this->_addressMode.value_or(VK_SAMPLER_ADDRESS_MODE_REPEAT);

    VkSamplerCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = addressMode,
            .addressModeV = addressMode,
            .addressModeW = addressMode,
            .mipLodBias = 0,
            .anisotropyEnable = this->_maxAnisotropy.has_value(),
            .maxAnisotropy = this->_maxAnisotropy.value_or(1),
            .compareEnable = this->_compareOp.has_value(),
            .compareOp = this->_compareOp.value_or(VK_COMPARE_OP_ALWAYS),
            .minLod = 0,
            .maxLod = 1,
            .borderColor = this->_borderColor.value_or(VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK),
            .unnormalizedCoordinates = VK_FALSE
    };

    return this->_vulkanObjectsAllocator->createSampler(&createInfo);
}
