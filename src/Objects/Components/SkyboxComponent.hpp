#ifndef OBJECTS_COMPONENTS_SKYBOXCOMPONENT_HPP
#define OBJECTS_COMPONENTS_SKYBOXCOMPONENT_HPP

#include <array>
#include <memory>

#include "src/Objects/Components/Component.hpp"
#include "src/Rendering/Constants.hpp"

class MeshResource;
class CubeImageResource;
class DescriptorSetObject;
class ImageViewObject;

class SkyboxComponent : public Component {
private:
    std::weak_ptr<MeshResource> _mesh;
    std::weak_ptr<CubeImageResource> _texture;

    std::array<std::shared_ptr<DescriptorSetObject>, MAX_INFLIGHT_FRAMES> _descriptorSets;
    std::shared_ptr<ImageViewObject> _textureView;

public:
    ~SkyboxComponent() override;

    [[nodiscard]] const std::weak_ptr<MeshResource> &mesh() const { return this->_mesh; }
    [[nodiscard]] const std::weak_ptr<CubeImageResource> &texture() const { return this->_texture; }

    void setMesh(const std::weak_ptr<MeshResource> &mesh);
    void setTexture(const std::weak_ptr<CubeImageResource> &texture);

    [[nodiscard]] std::array<std::shared_ptr<DescriptorSetObject>, MAX_INFLIGHT_FRAMES> &descriptorSets() {
        return this->_descriptorSets;
    }
    [[nodiscard]] std::shared_ptr<ImageViewObject> &textureView() { return this->_textureView; }
};


#endif // OBJECTS_COMPONENTS_SKYBOXCOMPONENT_HPP
