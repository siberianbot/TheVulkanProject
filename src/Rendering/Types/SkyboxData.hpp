#ifndef RENDERING_TYPES_SKYBOXDATA_HPP
#define RENDERING_TYPES_SKYBOXDATA_HPP

#include <memory>

class BufferObject;
class DescriptorSetObject;

struct SkyboxData {
    std::shared_ptr<BufferObject> vertices;
    std::shared_ptr<BufferObject> indices;
    uint32_t count;
    std::shared_ptr<DescriptorSetObject> descriptorSet;
};

#endif // RENDERING_TYPES_SKYBOXDATA_HPP
