#include "MeshResource.hpp"

#include <map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "src/Rendering/RendererAllocator.hpp"
#include "src/Rendering/Objects/BufferObject.hpp"

std::tuple<std::vector<uint32_t>, std::vector<Vertex>> MeshResource::loadData() {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, this->_paths[0].c_str())) {
        throw std::runtime_error("Failed to load mesh data");
    }

    // TODO: indexes are ignored
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    for (const tinyobj::shape_t &shape: shapes) {
        for (const tinyobj::index_t &index: shape.mesh.indices) {
            indices.push_back(vertices.size());

            Vertex vertex{
                    .pos = {
                            attrib.vertices[3 * index.vertex_index + 0],
                            attrib.vertices[3 * index.vertex_index + 1],
                            attrib.vertices[3 * index.vertex_index + 2]
                    },
                    .color = {1, 1, 1}
            };

            uint32_t normalBaseIdx = 3 * index.normal_index;
            if (normalBaseIdx < attrib.normals.size()) {
                vertex.normal = {
                        attrib.normals[normalBaseIdx + 0],
                        attrib.normals[normalBaseIdx + 1],
                        attrib.normals[normalBaseIdx + 2]
                };
            }

            uint32_t uvBaseIdx = 2 * index.texcoord_index;
            if (uvBaseIdx < attrib.texcoords.size()) {
                vertex.uv = {
                        attrib.texcoords[uvBaseIdx + 0],
                        1 - attrib.texcoords[uvBaseIdx + 1]
                };
            }

            vertices.push_back(vertex);
        }
    }

    return std::make_tuple(indices, vertices);
}

MeshResource::MeshResource(const std::filesystem::path &path, RendererAllocator *rendererAllocator)
        : Resource({path}),
          _rendererAllocator(rendererAllocator) {
    //
}

void MeshResource::load() {
    if (this->_isLoaded) {
        return;
    }

    const auto [indices, vertices] = this->loadData();

    this->_vertexBuffer = this->_rendererAllocator->uploadVertices(vertices);
    this->_indexBuffer = this->_rendererAllocator->uploadIndices(indices);
    this->_count = indices.size();

    Resource::load();
}

void MeshResource::unload() {
    if (!this->_isLoaded) {
        return;
    }

    if (this->_vertexBuffer != nullptr) {
        delete this->_vertexBuffer;
        this->_vertexBuffer = nullptr;
    }

    if (this->_indexBuffer != nullptr) {
        delete this->_indexBuffer;
        this->_indexBuffer = nullptr;
    }

    this->_count = 0;

    Resource::unload();
}
