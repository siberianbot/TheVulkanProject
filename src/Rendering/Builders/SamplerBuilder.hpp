#ifndef RENDERING_BUILDERS_SAMPLERBUILDER_HPP
#define RENDERING_BUILDERS_SAMPLERBUILDER_HPP

#include <memory>
#include <optional>

#include <vulkan/vulkan.hpp>

class RenderingDevice;

class SamplerBuilder {
private:
    std::shared_ptr<RenderingDevice> _renderingDevice;

    std::optional<float> _maxAnisotropy;
    std::optional<VkCompareOp> _compareOp;
    std::optional<VkSamplerAddressMode> _addressMode;
    std::optional<VkBorderColor> _borderColor;

public:
    SamplerBuilder(const std::shared_ptr<RenderingDevice> &renderingDevice);

    SamplerBuilder &enableAnisotropy(float maxAnisotropy);
    SamplerBuilder &enableComparison(VkCompareOp compareOp);

    SamplerBuilder &withAddressMode(VkSamplerAddressMode addressMode);
    SamplerBuilder &withBorderColor(VkBorderColor borderColor);

    VkSampler build();
};

#endif // RENDERING_BUILDERS_SAMPLERBUILDER_HPP
