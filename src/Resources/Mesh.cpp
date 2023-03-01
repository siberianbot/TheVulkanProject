#include "Mesh.hpp"

#include <map>
#include <stdexcept>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

Mesh Mesh::fromFile(const std::string &path) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
        throw std::runtime_error("mesh loading failure");
    }

    Mesh mesh;
    std::map<uint32_t, Vertex> indexMap;

    for (const auto &shape: shapes) {
        for (const auto &index: shape.mesh.indices) {
            mesh._indices.push_back(index.vertex_index);

            if (indexMap.find(index.vertex_index) != indexMap.end()) {
                continue;
            }

            indexMap.emplace(index.vertex_index, Vertex{
                    .pos = {
                            attrib.vertices[3 * index.vertex_index + 0],
                            attrib.vertices[3 * index.vertex_index + 1],
                            attrib.vertices[3 * index.vertex_index + 2]
                    },
                    .normal = {
                            attrib.normals[3 * index.normal_index + 0],
                            attrib.normals[3 * index.normal_index + 1],
                            attrib.normals[3 * index.normal_index + 2]
                    },
                    .color = {1, 1, 1},
                    .uv = {
                            attrib.texcoords[2 * index.texcoord_index + 0],
                            1 - attrib.texcoords[2 * index.texcoord_index + 1]
                    }
            });
        }
    }

    mesh._vertices.resize(indexMap.size());
    for (const auto &pair: indexMap) {
        mesh._vertices[pair.first] = pair.second;
    }

    return mesh;
}
