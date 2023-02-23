#include "DescriptorSetObject.hpp"

#include "src/Rendering/RenderingDevice.hpp"

DescriptorSetObject::DescriptorSetObject(RenderingDevice *renderingDevice, VkDescriptorPool descriptorPool,
                                         const std::array<VkDescriptorSet, MAX_INFLIGHT_FRAMES> &descriptorSets)
        : _renderingDevice(renderingDevice),
          _descriptorPool(descriptorPool),
          _descriptorSets(descriptorSets) {
    //
}

DescriptorSetObject::~DescriptorSetObject() {
    this->_renderingDevice->freeDescriptorSets(this->_descriptorPool, this->_descriptorSets);
}
