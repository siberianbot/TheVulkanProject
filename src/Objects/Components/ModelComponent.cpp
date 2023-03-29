#include "ModelComponent.hpp"

#include "src/Debug/UI/ObjectEditVisitor.hpp"
#include "src/Rendering/Objects/DescriptorSetObject.hpp"
#include "src/Rendering/Objects/ImageViewObject.hpp"

ModelComponent::~ModelComponent() {
    for (const auto &descriptorSet: this->_descriptorSets) {
        if (descriptorSet == nullptr) {
            continue;
        }

        descriptorSet->destroy();
    }

    if (this->_albedoTextureView != nullptr) {
        this->_albedoTextureView->destroy();
    }

    if (this->_specularTextureView != nullptr) {
        this->_specularTextureView->destroy();
    }
}

void ModelComponent::setMesh(const std::shared_ptr<MeshResource> &mesh) {
    this->_mesh = mesh;
}

void ModelComponent::setAlbedoTexture(const std::shared_ptr<ImageResource> &texture) {
    this->_albedoTexture = texture;
    this->_dirty = true;
}

void ModelComponent::setSpecularTexture(const std::shared_ptr<ImageResource> &texture) {
    this->_specularTexture = texture;
    this->_dirty = true;
}

void ModelComponent::acceptEdit(const std::shared_ptr<ObjectEditVisitor> &visitor) {
    visitor->drawModelComponent(this);
}