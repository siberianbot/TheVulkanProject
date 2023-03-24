#ifndef RENDERING_CONSTANTS_HPP
#define RENDERING_CONSTANTS_HPP

#include <array>

#include <glm/vec3.hpp>

static constexpr const uint32_t MAX_INFLIGHT_FRAMES = 2;

static constexpr std::array<glm::vec3, 6> POINT_LIGHT_SOURCE_DIRECTIONS = {
        glm::vec3(1, 0, 0),
        glm::vec3(-1, 0, 0),
        glm::vec3(0, 1, 0),
        glm::vec3(0, -1, 0),
        glm::vec3(0, 0, 1),
        glm::vec3(0, 0, -1)
};

#endif // RENDERING_CONSTANTS_HPP
