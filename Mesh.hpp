#ifndef MESH_HPP
#define MESH_HPP

#include <vector>
#include <string>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

Mesh readMesh(const std::string &path);

#endif // MESH_HPP
