#ifndef RENDERING_TYPES_SHADOWDATA_HPP
#define RENDERING_TYPES_SHADOWDATA_HPP

#include <glm/mat4x4.hpp>

struct ShadowData {
    alignas(16) glm::mat4 matrix;
};

#endif // RENDERING_TYPES_SHADOWDATA_HPP
