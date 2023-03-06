#ifndef RENDERING_OBJECTS_DESCRIPTORSETOBJECT_HPP
#define RENDERING_OBJECTS_DESCRIPTORSETOBJECT_HPP

#include <vulkan/vulkan.hpp>

#include "src/Rendering/Common.hpp"

class RenderingDevice;

class DescriptorSetObject {
private:
    RenderingDevice *_renderingDevice;

    VkDescriptorPool _descriptorPool;
    std::vector<VkDescriptorSet> _descriptorSets;

public:
    DescriptorSetObject(RenderingDevice *renderingDevice, VkDescriptorPool descriptorPool,
                        const std::vector<VkDescriptorSet> &descriptorSets);
    ~DescriptorSetObject();

    [[nodiscard]] VkDescriptorSet getDescriptorSet(uint32_t idx) const { return this->_descriptorSets[idx]; }
};


#endif // RENDERING_OBJECTS_DESCRIPTORSETOBJECT_HPP
