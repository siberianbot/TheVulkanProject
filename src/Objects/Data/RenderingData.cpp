#include "RenderingData.hpp"

#include "src/Rendering/Objects/DescriptorSetObject.hpp"

RenderingData::~RenderingData() {
    for (std::shared_ptr<DescriptorSetObject> &descriptorSet: this->descriptorSets) {
        descriptorSet->destroy();
    }
}
