#ifndef OBJECTS_COMPONENTS_MODELCOMPONENT_HPP
#define OBJECTS_COMPONENTS_MODELCOMPONENT_HPP

#include <array>
#include <memory>
#include <optional>

#include "src/Objects/Components/Component.hpp"
#include "src/Resources/ResourceId.hpp"

class ModelComponent : public Component {
private:
    std::optional<ResourceId> _meshId;
    std::optional<ResourceId> _albedoTextureId;
    std::optional<ResourceId> _specularTextureId;

public:
    ~ModelComponent() override;

    void setMeshId(const std::optional<ResourceId> &meshId);
    void setAlbedoTextureId(const std::optional<ResourceId> &textureId);
    void setSpecularTextureId(const std::optional<ResourceId> &textureId);

    [[nodiscard]] const std::optional<ResourceId> &meshId() const { return this->_meshId; }

    [[nodiscard]] const std::optional<ResourceId> &albedoTextureId() const { return this->_albedoTextureId; }

    [[nodiscard]] const std::optional<ResourceId> &specularTextureId() const { return this->_specularTextureId; }

    void acceptEdit(const std::shared_ptr<ObjectEditVisitor> &visitor) override;
};

#endif // OBJECTS_COMPONENTS_MODELCOMPONENT_HPP
