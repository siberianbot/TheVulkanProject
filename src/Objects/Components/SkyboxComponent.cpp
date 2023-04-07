#include "SkyboxComponent.hpp"

#include "src/Debug/UI/ObjectEditVisitor.hpp"
#include "src/Rendering/Objects/DescriptorSetObject.hpp"

SkyboxComponent::~SkyboxComponent() {
    for (const auto &descriptorSet: this->_descriptorSets) {
        if (descriptorSet == nullptr) {
            continue;
        }

        descriptorSet->destroy();
    }
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
