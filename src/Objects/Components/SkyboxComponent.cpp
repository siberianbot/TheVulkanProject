#include "SkyboxComponent.hpp"

#include "src/Debug/UI/ObjectEditVisitor.hpp"

SkyboxComponent::~SkyboxComponent() {
    // TODO
}

void SkyboxComponent::setMeshId(const std::optional<ResourceId> &meshId) {
    this->_meshId = meshId;
}

void SkyboxComponent::setTextureIds(const std::array<ResourceId, SKYBOX_TEXTURE_ARRAY_SIZE> &textureIds) {
    this->_textureIds = textureIds;
    this->_dirty = true;
}

void SkyboxComponent::acceptEdit(const std::shared_ptr<ObjectEditVisitor> &visitor) {
    visitor->drawSkyboxComponent(this);
}
