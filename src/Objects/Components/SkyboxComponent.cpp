#include "SkyboxComponent.hpp"

#include "src/Debug/UI/ObjectEditVisitor.hpp"
#include "src/Rendering/Objects/DescriptorSetObject.hpp"
#include "src/Rendering/Objects/ImageViewObject.hpp"

SkyboxComponent::~SkyboxComponent() {
    for (const auto &descriptorSet: this->_descriptorSets) {
        if (descriptorSet == nullptr) {
            continue;
        }

        descriptorSet->destroy();
    }

    if (this->_textureView != nullptr) {
        this->_textureView->destroy();
    }
}

void SkyboxComponent::setMesh(const std::weak_ptr<MeshResource> &mesh) {
    this->_mesh = mesh;
}

void SkyboxComponent::setTexture(const std::weak_ptr<CubeImageResource> &texture) {
    this->_texture = texture;
    this->_dirty = true;
}

void SkyboxComponent::acceptEdit(const std::shared_ptr<ObjectEditVisitor> &visitor) {
    visitor->drawSkyboxComponent(this);
}
