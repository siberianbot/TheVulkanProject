#ifndef RENDERING_BUILDERS_DESCRIPTORPOOLBUILDER_HPP
#define RENDERING_BUILDERS_DESCRIPTORPOOLBUILDER_HPP

#include <memory>
#include <optional>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "src/Rendering/Constants.hpp"

class VulkanObjectsAllocator;

class DescriptorPoolBuilder {
private:
    std::shared_ptr<VulkanObjectsAllocator> _vulkanObjectsAllocator;

    std::optional<uint32_t> _setCount;
    std::vector<VkDescriptorPoolSize> _sizes;

public:
    DescriptorPoolBuilder(const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator);

    DescriptorPoolBuilder &withSetCount(uint32_t count);
    DescriptorPoolBuilder &forType(VkDescriptorType type, uint32_t count = DEFAULT_TYPE_COUNT);

    VkDescriptorPool build();
};


#endif // RENDERING_BUILDERS_DESCRIPTORPOOLBUILDER_HPP
