#ifndef RENDERING_TYPES_MESHRENDERINGDATA_HPP
#define RENDERING_TYPES_MESHRENDERINGDATA_HPP

#include <memory>

class BufferObject;

struct MeshRenderingData {
    std::shared_ptr<BufferObject> vertexBuffer;
    std::shared_ptr<BufferObject> indexBuffer;

    [[nodiscard]] bool isValid() {
        return this->vertexBuffer != nullptr && this->indexBuffer != nullptr;
    }

    void free();
};

#endif // RENDERING_TYPES_MESHRENDERINGDATA_HPP
