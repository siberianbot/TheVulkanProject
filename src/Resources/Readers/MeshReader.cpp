#include "MeshReader.hpp"

#include <fmt/core.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "src/Engine/EngineError.hpp"
#include "src/Engine/Log.hpp"
#include "src/Resources/ResourceData.hpp"
#include "src/Utils/DataStream.hpp"

static constexpr const char *MESH_READER_TAG = "MeshReader";

std::unique_ptr<MeshData> MeshReader::read(const std::weak_ptr<ResourceData> &resourceData) {
    std::shared_ptr<ResourceData> lockedResourceData = resourceData.lock();
    DataStream stream = lockedResourceData->stream();

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warning;
    std::string error;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &error, &stream) ||
        !error.empty()) {
        throw EngineError(fmt::format("Failed to read mesh data: {0}", error));
    }

    if (!warning.empty()) {
        this->_log->warning(MESH_READER_TAG, fmt::format("Mesh data reading warning: {0}", warning));
    }

    std::unique_ptr<MeshData> meshData = std::make_unique<MeshData>();

    for (const tinyobj::shape_t &shape: shapes) {
        for (const tinyobj::index_t &index: shape.mesh.indices) {
            meshData->indices.push_back(meshData->vertices.size());

            Vertex vertex = {
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

            meshData->vertices.push_back(vertex);
        }
    }

    return meshData;
}

MeshReader::MeshReader(const std::shared_ptr<Log> &log)
        : _log(log) {
    //
}

std::optional<std::unique_ptr<MeshData>> MeshReader::tryRead(const std::weak_ptr<ResourceData> &resourceData) {
    try {
        return this->read(resourceData);
    } catch (const std::exception &error) {
        this->_log->error(MESH_READER_TAG, error);
        return std::nullopt;
    }
}
