#ifndef RENDERING_BUILDERS_SAMPLERBUILDER_HPP
#define RENDERING_BUILDERS_SAMPLERBUILDER_HPP

#include <memory>
#include <optional>

#include <vulkan/vulkan.hpp>

class VulkanObjectsAllocator;

class SamplerBuilder {
private:
    std::shared_ptr<VulkanObjectsAllocator> _vulkanObjectsAllocator;

    std::optional<float> _maxAnisotropy;
    std::optional<VkCompareOp> _compareOp;
    std::optional<VkSamplerAddressMode> _addressMode;
    std::optional<VkBorderColor> _borderColor;

public:
    SamplerBuilder(const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator);

    SamplerBuilder &enableAnisotropy(float maxAnisotropy);
    SamplerBuilder &enableComparison(VkCompareOp compareOp);

    SamplerBuilder &withAddressMode(VkSamplerAddressMode addressMode);
    SamplerBuilder &withBorderColor(VkBorderColor borderColor);

    VkSampler build();
};

#endif // RENDERING_BUILDERS_SAMPLERBUILDER_HPP
