#ifndef OBJECTS_COMPONENTS_MODELCOMPONENT_HPP
#define OBJECTS_COMPONENTS_MODELCOMPONENT_HPP

#include <array>
#include <memory>

#include "src/Objects/Components/Component.hpp"
#include "src/Rendering/Constants.hpp"

class MeshResource;
class ImageResource;
class DescriptorSetObject;
class ImageViewObject;

class ModelComponent : public Component {
private:
    std::weak_ptr<MeshResource> _mesh;
    std::weak_ptr<ImageResource> _albedoTexture;
    std::weak_ptr<ImageResource> _specularTexture;

    std::array<std::shared_ptr<DescriptorSetObject>, MAX_INFLIGHT_FRAMES> _descriptorSets;
    std::shared_ptr<ImageViewObject> _albedoTextureView;
    std::shared_ptr<ImageViewObject> _specularTextureView;

public:
    ~ModelComponent() override;

    [[nodiscard]] const std::weak_ptr<MeshResource> &mesh() const { return this->_mesh; }
    [[nodiscard]] const std::weak_ptr<ImageResource> &albedoTexture() const { return this->_albedoTexture; }
    [[nodiscard]] const std::weak_ptr<ImageResource> &specularTexture() const { return this->_specularTexture; }

    void setMesh(const std::shared_ptr<MeshResource> &mesh);
    void setAlbedoTexture(const std::shared_ptr<ImageResource> &texture);
    void setSpecularTexture(const std::shared_ptr<ImageResource> &texture);

    [[nodiscard]] std::array<std::shared_ptr<DescriptorSetObject>, MAX_INFLIGHT_FRAMES> &descriptorSets() {
        return this->_descriptorSets;
    }
    [[nodiscard]] std::shared_ptr<ImageViewObject> &albedoTextureView() { return this->_albedoTextureView; }
    [[nodiscard]] std::shared_ptr<ImageViewObject> &specularTextureView() { return this->_specularTextureView; }

    void acceptEdit(const std::shared_ptr<ObjectEditVisitor> &visitor) override;
};

#endif // OBJECTS_COMPONENTS_MODELCOMPONENT_HPP
