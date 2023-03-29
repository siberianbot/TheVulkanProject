#ifndef RENDERING_TYPES_MODELDATA_HPP
#define RENDERING_TYPES_MODELDATA_HPP

#include <memory>

#include <glm/mat4x4.hpp>

class BufferObject;
class DescriptorSetObject;

struct ModelData {
    glm::mat4 model;
    glm::mat4 modelRotation;
    std::shared_ptr<BufferObject> vertices;
    std::shared_ptr<BufferObject> indices;
    uint32_t count;
    std::shared_ptr<DescriptorSetObject> descriptorSet;
};

#endif // RENDERING_TYPES_MODELDATA_HPP