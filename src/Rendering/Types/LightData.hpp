#ifndef RENDERING_TYPES_LIGHTDATA_HPP
#define RENDERING_TYPES_LIGHTDATA_HPP

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

struct LightData {
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec3 color;
    alignas(4) float range;
};

#endif // RENDERING_TYPES_LIGHTDATA_HPP
