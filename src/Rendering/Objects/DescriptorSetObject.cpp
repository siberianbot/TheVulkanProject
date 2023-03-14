#include "DescriptorSetObject.hpp"

#include "src/Rendering/RenderingDevice.hpp"

DescriptorSetObject::DescriptorSetObject(RenderingDevice *renderingDevice, VkDescriptorPool descriptorPool,
                                         const std::vector<VkDescriptorSet> &descriptorSets)
        : _renderingDevice(renderingDevice),
          _descriptorPool(descriptorPool),
          _descriptorSets(descriptorSets) {
    //
}

DescriptorSetObject::~DescriptorSetObject() {
    this->_renderingDevice->freeDescriptorSets(this->_descriptorPool, this->_descriptorSets.size(),
                                               this->_descriptorSets.data());
}

DescriptorSetObject *DescriptorSetObject::create(RenderingDevice *renderingDevice, uint32_t count,
                                                 VkDescriptorPool pool, VkDescriptorSetLayout layout) {
    std::vector<VkDescriptorSet> descripotSets = renderingDevice->allocateDescriptorSets(count, pool, layout);

    return new DescriptorSetObject(renderingDevice, pool, descripotSets);
}
