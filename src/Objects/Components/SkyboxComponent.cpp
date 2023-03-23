#include "SkyboxComponent.hpp"

#include "src/Rendering/Objects/DescriptorSetObject.hpp"

SkyboxComponent::~SkyboxComponent() {
    for (const auto &descriptorSet: this->_descriptorSets) {
        if (descriptorSet == nullptr) {
            continue;
        }

        descriptorSet->destroy();
    }
}

void SkyboxComponent::setMesh(const std::weak_ptr<MeshResource> &mesh) {
    this->_mesh = mesh;
}

void SkyboxComponent::setTexture(const std::weak_ptr<CubeImageResource> &texture) {
    this->_texture = texture;
    this->_dirty = true;
}
