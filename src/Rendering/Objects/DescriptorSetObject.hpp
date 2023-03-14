#ifndef RENDERING_OBJECTS_DESCRIPTORSETOBJECT_HPP
#define RENDERING_OBJECTS_DESCRIPTORSETOBJECT_HPP

#include <vulkan/vulkan.hpp>

class RenderingDevice;

class DescriptorSetObject {
private:
    RenderingDevice *_renderingDevice;

    VkDescriptorPool _descriptorPool;
    std::vector<VkDescriptorSet> _descriptorSets;

    DescriptorSetObject(RenderingDevice *renderingDevice, VkDescriptorPool descriptorPool,
                        const std::vector<VkDescriptorSet> &descriptorSets);

public:
    ~DescriptorSetObject();

    [[nodiscard]] VkDescriptorSet getDescriptorSet(uint32_t idx) const { return this->_descriptorSets[idx]; }

    [[nodiscard]] static DescriptorSetObject *create(RenderingDevice *renderingDevice, uint32_t count,
                                                     VkDescriptorPool pool, VkDescriptorSetLayout layout);
};


#endif // RENDERING_OBJECTS_DESCRIPTORSETOBJECT_HPP
