#ifndef RENDERING_TYPES_MESHCONSTANTS_HPP
#define RENDERING_TYPES_MESHCONSTANTS_HPP

#include <glm/mat4x4.hpp>

struct MeshConstants {
    glm::mat4 matrix;
    glm::mat4 model;
    glm::mat4 modelRotation;
};

#endif // RENDERING_TYPES_MESHCONSTANTS_HPP
