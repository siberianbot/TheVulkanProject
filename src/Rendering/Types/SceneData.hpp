#ifndef RENDERING_TYPES_SCENEDATA_HPP
#define RENDERING_TYPES_SCENEDATA_HPP

#include <cstdint>

struct SceneData {
    alignas(4) float ambient;
    alignas(4) uint32_t shadowCount;
    alignas(4) uint32_t lightCount;
};

#endif // RENDERING_TYPES_SCENEDATA_HPP
