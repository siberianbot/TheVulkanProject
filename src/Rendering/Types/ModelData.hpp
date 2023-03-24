#ifndef RENDERING_TYPES_MODELDATA_HPP
#define RENDERING_TYPES_MODELDATA_HPP

#include <memory>

#include <glm/mat4x4.hpp>

class BufferObject;

struct ModelData {
    glm::mat4 model;
    glm::mat4 modelRotation;
    std::shared_ptr<BufferObject> vertices;
    std::shared_ptr<BufferObject> indices;
    uint32_t count;
};

#endif // RENDERING_TYPES_MODELDATA_HPP
