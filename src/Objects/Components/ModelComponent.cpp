#include "ModelComponent.hpp"

#include "src/Rendering/Objects/DescriptorSetObject.hpp"

ModelComponent::~ModelComponent() {
    for (const auto &descriptorSet: this->descriptorSets) {
        descriptorSet->destroy();
    }
}
