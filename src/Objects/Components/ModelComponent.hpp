#ifndef OBJECTS_COMPONENTS_MODELCOMPONENT_HPP
#define OBJECTS_COMPONENTS_MODELCOMPONENT_HPP

#include <array>
#include <memory>

#include "src/Objects/Components/IComponent.hpp"
#include "src/Rendering/Constants.hpp"

class MeshResource;
class ImageResource;
class DescriptorSetObject;
class ImageViewObject;

struct ModelComponent : public IComponent {
public:
    ~ModelComponent() override;

    std::weak_ptr<MeshResource> mesh;
    std::weak_ptr<ImageResource> albedoTexture;
    std::weak_ptr<ImageResource> specularTexture;

    std::array<std::shared_ptr<DescriptorSetObject>, MAX_INFLIGHT_FRAMES> descriptorSets;
    std::shared_ptr<ImageViewObject> albedoTextureView;
    std::shared_ptr<ImageViewObject> specularTextureView;
};

#endif // OBJECTS_COMPONENTS_MODELCOMPONENT_HPP
