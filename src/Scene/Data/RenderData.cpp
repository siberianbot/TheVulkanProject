#include "RenderData.hpp"

#include "src/Rendering/Objects/DescriptorSetObject.hpp"

RenderData::~RenderData() {
    delete descriptorSet;
}
