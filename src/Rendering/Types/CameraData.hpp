#ifndef RENDERING_TYPES_CAMERADATA_HPP
#define RENDERING_TYPES_CAMERADATA_HPP

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

struct CameraData {
    alignas(16) glm::mat4 matrixAll;
    alignas(16) glm::mat4 matrixRot;
    alignas(16) glm::vec3 position;
};

#endif // RENDERING_TYPES_CAMERADATA_HPP
