#ifndef RESOURCES_MESHRESOURCE_HPP
#define RESOURCES_MESHRESOURCE_HPP

#include <cstdint>
#include <vector>

#include "Resource.hpp"
#include "src/Types/Vertex.hpp"

class RendererAllocator;
class BufferObject;

class MeshResource : public Resource {
private:
    std::shared_ptr<RendererAllocator> _rendererAllocator;

    std::shared_ptr<BufferObject> _vertexBuffer = nullptr;
    std::shared_ptr<BufferObject> _indexBuffer = nullptr;
    uint32_t _count = 0;

    std::tuple<std::vector<uint32_t>, std::vector<Vertex>> loadData();

public:
    MeshResource(const std::string &id, const std::filesystem::path &path,
                 const std::shared_ptr<RendererAllocator> &rendererAllocator);
    ~MeshResource() override = default;

    void load() override;
    void unload() override;

    [[nodiscard]] ResourceType type() const override { return MESH_RESOURCE; }
    [[nodiscard]] std::shared_ptr<BufferObject> vertexBuffer() const { return this->_vertexBuffer; }
    [[nodiscard]] std::shared_ptr<BufferObject> indexBuffer() const { return this->_indexBuffer; }
    [[nodiscard]] uint32_t count() const { return this->_count; }
};

#endif // RESOURCES_MESHRESOURCE_HPP
