#ifndef RENDERING_OBJECTS_DESCRIPTORSETOBJECT_HPP
#define RENDERING_OBJECTS_DESCRIPTORSETOBJECT_HPP

#include <vulkan/vulkan.hpp>

#include "Rendering/Common.hpp"

class RenderingDevice;

class DescriptorSetObject {
private:
    RenderingDevice *_renderingDevice;

    VkDescriptorPool _descriptorPool;
    std::array<VkDescriptorSet, MAX_INFLIGHT_FRAMES> _descriptorSets;

public:
    DescriptorSetObject(RenderingDevice *renderingDevice, VkDescriptorPool descriptorPool,
                        const std::array<VkDescriptorSet, MAX_INFLIGHT_FRAMES> &descriptorSets);
    ~DescriptorSetObject();

    [[nodiscard]] VkDescriptorSet getDescriptorSet(uint32_t frameIdx) const { return this->_descriptorSets[frameIdx]; }
};


#endif // RENDERING_OBJECTS_DESCRIPTORSETOBJECT_HPP
