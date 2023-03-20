#include "DescriptorSetObject.hpp"

#include "src/Rendering/Common.hpp"
#include "src/Rendering/RenderingDevice.hpp"

DescriptorSetObject::DescriptorSetObject(const std::shared_ptr<RenderingDevice> &renderingDevice,
                                         VkDescriptorPool descriptorPool, VkDescriptorSet descriptorSet)
        : _renderingDevice(renderingDevice),
          _descriptorPool(descriptorPool),
          _descriptorSet(descriptorSet) {
    //
}

void DescriptorSetObject::destroy() {
    vkFreeDescriptorSets(this->_renderingDevice->getHandle(), this->_descriptorPool, 1, &this->_descriptorSet);
}

std::shared_ptr<DescriptorSetObject> DescriptorSetObject::create(
        const std::shared_ptr<RenderingDevice> &renderingDevice, VkDescriptorPool descriptorPool,
        VkDescriptorSetLayout layout) {
    VkDescriptorSetAllocateInfo allocateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &layout
    };

    VkDescriptorSet descriptorSet;
    vkEnsure(vkAllocateDescriptorSets(renderingDevice->getHandle(), &allocateInfo, &descriptorSet));

    return std::make_shared<DescriptorSetObject>(renderingDevice, descriptorPool, descriptorSet);
}
