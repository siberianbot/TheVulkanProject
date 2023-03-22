#include "SkyboxComponent.hpp"

#include "src/Rendering/Objects/DescriptorSetObject.hpp"

SkyboxComponent::~SkyboxComponent() {
    for (const auto &descriptorSet: this->descriptorSets) {
        descriptorSet->destroy();
    }
}
