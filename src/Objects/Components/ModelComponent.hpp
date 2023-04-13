#ifndef OBJECTS_COMPONENTS_MODELCOMPONENT_HPP
#define OBJECTS_COMPONENTS_MODELCOMPONENT_HPP

#include <array>
#include <memory>
#include <optional>

#include "src/Objects/Components/Component.hpp"
#include "src/Rendering/Constants.hpp"
#include "src/Resources/ResourceId.hpp"

class DescriptorSetObject;

class ModelComponent : public Component {
private:
    std::optional<ResourceId> _meshId;
    std::optional<ResourceId> _albedoTextureId;
    std::optional<ResourceId> _specularTextureId;

    std::array<std::shared_ptr<DescriptorSetObject>, INFLIGHT_FRAMES> _descriptorSets;

public:
    ~ModelComponent() override;

    void setMeshId(const std::optional<ResourceId> &meshId);
    void setAlbedoTextureId(const std::optional<ResourceId> &textureId);
    void setSpecularTextureId(const std::optional<ResourceId> &textureId);

    [[nodiscard]] const std::optional<ResourceId> &meshId() const { return this->_meshId; }

    [[nodiscard]] const std::optional<ResourceId> &albedoTextureId() const { return this->_albedoTextureId; }

    [[nodiscard]] const std::optional<ResourceId> &specularTextureId() const { return this->_specularTextureId; }

    [[nodiscard]] std::array<std::shared_ptr<DescriptorSetObject>, INFLIGHT_FRAMES> &descriptorSets() {
        return this->_descriptorSets;
    }

    void acceptEdit(const std::shared_ptr<ObjectEditVisitor> &visitor) override;
};

#endif // OBJECTS_COMPONENTS_MODELCOMPONENT_HPP
