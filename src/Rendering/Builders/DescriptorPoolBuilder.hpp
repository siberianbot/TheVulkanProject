#ifndef RENDERING_BUILDERS_DESCRIPTORPOOLBUILDER_HPP
#define RENDERING_BUILDERS_DESCRIPTORPOOLBUILDER_HPP

#include <optional>
#include <vector>

#include <vulkan/vulkan.hpp>

class RenderingDevice;

static constexpr const uint32_t DEFAULT_SET_COUNT = 1000;
static constexpr const uint32_t DEFAULT_TYPE_COUNT = 1000;

class DescriptorPoolBuilder {
private:
    RenderingDevice *_renderingDevice;

    std::optional<uint32_t> _setCount;
    std::vector<VkDescriptorPoolSize> _sizes;

public:
    explicit DescriptorPoolBuilder(RenderingDevice *renderingDevice);

    DescriptorPoolBuilder &withSetCount(uint32_t count);
    DescriptorPoolBuilder &forType(VkDescriptorType type, uint32_t count = DEFAULT_TYPE_COUNT);

    VkDescriptorPool build();
};


#endif // RENDERING_BUILDERS_DESCRIPTORPOOLBUILDER_HPP
