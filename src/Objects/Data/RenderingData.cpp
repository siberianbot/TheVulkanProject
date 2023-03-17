#include "RenderingData.hpp"

#include "src/Rendering/Objects/DescriptorSetObject.hpp"

RenderingData::~RenderingData() {
    delete descriptorSet;
}
