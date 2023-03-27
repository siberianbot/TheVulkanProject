#ifndef RENDERING_TYPES_SHADOWDATA_HPP
#define RENDERING_TYPES_SHADOWDATA_HPP

#include <glm/mat4x4.hpp>

struct ShadowData {
    alignas(16) glm::mat4 matrix;
    alignas(16) glm::vec3 position;
    alignas(4) float range;
};

#endif // RENDERING_TYPES_SHADOWDATA_HPP
