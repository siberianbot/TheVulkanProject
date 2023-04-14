#include "ModelComponent.hpp"

#include "src/Debug/UI/ObjectEditVisitor.hpp"

ModelComponent::~ModelComponent() {
    // TODO
}

void ModelComponent::setMeshId(const std::optional<ResourceId> &meshId) {
    this->_meshId = meshId;
}

void ModelComponent::setAlbedoTextureId(const std::optional<ResourceId> &textureId) {
    this->_albedoTextureId = textureId;
    this->_dirty = true;
}

void ModelComponent::setSpecularTextureId(const std::optional<ResourceId> &textureId) {
    this->_specularTextureId = textureId;
    this->_dirty = true;
}

void ModelComponent::acceptEdit(const std::shared_ptr<ObjectEditVisitor> &visitor) {
    visitor->drawModelComponent(this);
}
