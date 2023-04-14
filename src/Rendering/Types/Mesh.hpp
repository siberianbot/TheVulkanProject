#ifndef RENDERING_TYPES_MESH_HPP
#define RENDERING_TYPES_MESH_HPP

#include <memory>

#include "src/Rendering/Types/BufferView.hpp"

struct Mesh {
    std::weak_ptr<BufferView> vertexBuffer;
    std::weak_ptr<BufferView> indexBuffer;
};

#endif // RENDERING_TYPES_MESH_HPP
