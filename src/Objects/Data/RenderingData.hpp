#ifndef OBJECTS_DATA_RENDERDATA_HPP
#define OBJECTS_DATA_RENDERDATA_HPP

#include <memory>

#include "IData.hpp"

class DescriptorSetObject;
class ImageViewObject;

struct RenderingData : public IData {
    RenderingData() = default;
    ~RenderingData() override;

    DescriptorSetObject *descriptorSet = nullptr;
    std::shared_ptr<ImageViewObject> albedoTextureView = nullptr;
    std::shared_ptr<ImageViewObject> specTextureView = nullptr;
};

#endif // OBJECTS_DATA_RENDERDATA_HPP
