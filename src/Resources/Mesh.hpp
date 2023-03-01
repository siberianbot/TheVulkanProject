#ifndef RESOURCES_MESH_HPP
#define RESOURCES_MESH_HPP

#include <vector>
#include <string>

#include "Vertex.hpp"

class Mesh {
private:
    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indices;

    Mesh() = default;

public:
    [[nodiscard]] std::vector<Vertex> &vertices() { return this->_vertices; }
    [[nodiscard]] std::vector<uint32_t> &indices() { return this->_indices; }

    [[nodiscard]] static Mesh fromFile(const std::string &path);
};

#endif // RESOURCES_MESH_HPP
