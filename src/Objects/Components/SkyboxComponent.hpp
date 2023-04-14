#ifndef OBJECTS_COMPONENTS_SKYBOXCOMPONENT_HPP
#define OBJECTS_COMPONENTS_SKYBOXCOMPONENT_HPP

#include <array>
#include <memory>
#include <optional>

#include "src/Objects/Components/Component.hpp"
#include "src/Resources/ResourceId.hpp"

static constexpr const uint32_t SKYBOX_TEXTURE_ARRAY_SIZE = 6;

class SkyboxComponent : public Component {
private:
    std::optional<ResourceId> _meshId;
    std::array<ResourceId, SKYBOX_TEXTURE_ARRAY_SIZE> _textureIds;

public:
    ~SkyboxComponent() override;

    void setMeshId(const std::optional<ResourceId> &meshId);
    void setTextureIds(const std::array<ResourceId, SKYBOX_TEXTURE_ARRAY_SIZE> &textureIds);

    [[nodiscard]] const std::optional<ResourceId> &meshId() const { return this->_meshId; }

    [[nodiscard]] const std::array<ResourceId, SKYBOX_TEXTURE_ARRAY_SIZE> &textureIds() const {
        return this->_textureIds;
    }

    void acceptEdit(const std::shared_ptr<ObjectEditVisitor> &visitor) override;
};


#endif // OBJECTS_COMPONENTS_SKYBOXCOMPONENT_HPP
