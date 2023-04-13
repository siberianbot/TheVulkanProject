#include "DescriptorSetObject.hpp"

#include "src/Rendering/Common.hpp"
#include "src/Rendering/RenderingDevice.hpp"
#include "src/Rendering/Objects/BufferObject.hpp"
#include "src/Rendering/Objects/ImageViewObject.hpp"

DescriptorSetObject::DescriptorSetObject(const std::shared_ptr<RenderingDevice> &renderingDevice,
                                         VkDescriptorPool descriptorPool, VkDescriptorSet descriptorSet)
        : _renderingDevice(renderingDevice),
          _descriptorPool(descriptorPool),
          _descriptorSet(descriptorSet) {
    //
}

void DescriptorSetObject::updateWithBuffer(uint32_t bindingIdx, VkDescriptorType descriptorType,
                                           const std::shared_ptr<BufferObject> buffer, uint32_t offset,
                                           uint32_t range) {
    VkDescriptorBufferInfo bufferInfo = {
            .buffer = buffer->getHandle(),
            .offset = offset,
            .range = range
    };

    VkWriteDescriptorSet write = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = this->_descriptorSet,
            .dstBinding = bindingIdx,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = descriptorType,
            .pImageInfo = nullptr,
            .pBufferInfo = &bufferInfo,
            .pTexelBufferView = nullptr
    };

    vkUpdateDescriptorSets(this->_renderingDevice->getHandle(), 1, &write, 0, nullptr);

    this->_bindings[bindingIdx] = DescriptorSetBinding{.buffer = buffer};
}

void DescriptorSetObject::updateWithImageView(uint32_t bindingIdx, VkDescriptorType descriptorType,
                                              const std::shared_ptr<ImageViewObject> imageView,
                                              VkImageLayout imageLayout, VkSampler sampler) {
    VkDescriptorImageInfo imageInfo = {
            .sampler = sampler,
            .imageView = imageView->getHandle(),
            .imageLayout = imageLayout
    };

    VkWriteDescriptorSet write = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = this->_descriptorSet,
            .dstBinding = bindingIdx,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = descriptorType,
            .pImageInfo = &imageInfo,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
    };

    vkUpdateDescriptorSets(this->_renderingDevice->getHandle(), 1, &write, 0, nullptr);

    this->_bindings[bindingIdx] = DescriptorSetBinding{.imageView = imageView};
}

void DescriptorSetObject::destroy() {
    this->_bindings.clear();

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
