#include "MeshRenderingData.hpp"

#include "src/Rendering/Objects/BufferObject.hpp"

void MeshRenderingData::free() {
    this->vertexBuffer->destroy();
    this->vertexBuffer = nullptr;

    this->indexBuffer->destroy();
    this->indexBuffer = nullptr;
}
