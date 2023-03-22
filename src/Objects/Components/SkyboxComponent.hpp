#ifndef SRC_OBJECTS_COMPONENTS_SKYBOXCOMPONENT_HPP
#define SRC_OBJECTS_COMPONENTS_SKYBOXCOMPONENT_HPP

#include <array>
#include <memory>

#include "src/Objects/Components/IComponent.hpp"
#include "src/Rendering/Constants.hpp"

class MeshResource;
class CubeImageResource;
class DescriptorSetObject;
class ImageViewObject;

struct SkyboxComponent : public IComponent {
public:
    ~SkyboxComponent() override;

    std::weak_ptr<MeshResource> mesh;
    std::weak_ptr<CubeImageResource> texture;

    std::array<std::shared_ptr<DescriptorSetObject>, MAX_INFLIGHT_FRAMES> descriptorSets;
    std::shared_ptr<ImageViewObject> textureView;
};


#endif // SRC_OBJECTS_COMPONENTS_SKYBOXCOMPONENT_HPP
