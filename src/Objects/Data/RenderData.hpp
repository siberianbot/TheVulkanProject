#ifndef OBJECTS_DATA_RENDERDATA_HPP
#define OBJECTS_DATA_RENDERDATA_HPP

#include "IData.hpp"

class DescriptorSetObject;
class ImageViewObject;

struct RenderData : public IData {
    RenderData() = default;
    ~RenderData() override;

    DescriptorSetObject *descriptorSet = nullptr;
    ImageViewObject *albedoTextureView = nullptr;
    ImageViewObject *specTextureView = nullptr;
};

#endif // OBJECTS_DATA_RENDERDATA_HPP
