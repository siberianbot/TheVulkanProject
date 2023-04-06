#ifndef RESOURCES_READERS_MESHREADER_HPP
#define RESOURCES_READERS_MESHREADER_HPP

#include <memory>
#include <optional>
#include <vector>

#include "src/Types/Vertex.hpp"

class Log;
class ResourceData;

struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

class MeshReader {
private:
    std::shared_ptr<Log> _log;

    std::unique_ptr<MeshData> read(const std::weak_ptr<ResourceData> &resourceData);

public:
    MeshReader(const std::shared_ptr<Log> &log);

    [[nodiscard]] std::optional<std::unique_ptr<MeshData>> tryRead(const std::weak_ptr<ResourceData> &resourceData);
};

#endif // RESOURCES_READERS_MESHREADER_HPP
