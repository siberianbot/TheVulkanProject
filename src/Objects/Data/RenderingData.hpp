#ifndef OBJECTS_DATA_RENDERDATA_HPP
#define OBJECTS_DATA_RENDERDATA_HPP

#include <array>
#include <memory>

#include "src/Objects/Data/IData.hpp"
#include "src/Rendering/Constants.hpp"

class DescriptorSetObject;
class ImageViewObject;

struct RenderingData : public IData {
    RenderingData() = default;
    ~RenderingData() override;

    std::array<std::shared_ptr<DescriptorSetObject>, MAX_INFLIGHT_FRAMES> descriptorSets;
    std::shared_ptr<ImageViewObject> albedoTextureView = nullptr;
    std::shared_ptr<ImageViewObject> specTextureView = nullptr;
};

#endif // OBJECTS_DATA_RENDERDATA_HPP
