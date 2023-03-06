#include "DescriptorPoolBuilder.hpp"

#include "src/Rendering/RenderingDevice.hpp"

DescriptorPoolBuilder::DescriptorPoolBuilder(RenderingDevice *renderingDevice)
        : _renderingDevice(renderingDevice) {
    //
}

DescriptorPoolBuilder &DescriptorPoolBuilder::withSetCount(uint32_t count) {
    this->_setCount = count;

    return *this;
}

DescriptorPoolBuilder &DescriptorPoolBuilder::forType(VkDescriptorType type, uint32_t count) {
    this->_sizes.push_back(VkDescriptorPoolSize{
            .type = type,
            .descriptorCount = count
    });

    return *this;
}

VkDescriptorPool DescriptorPoolBuilder::build() {
    return this->_renderingDevice->createDescriptorPool(this->_sizes, this->_setCount.value_or(DEFAULT_SET_COUNT));
}
