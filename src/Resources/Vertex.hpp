#ifndef RESOURCES_VERTEX_HPP
#define RESOURCES_VERTEX_HPP

#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 uv;
};

#endif // RESOURCES_VERTEX_HPP
