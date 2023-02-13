#ifndef MATH_HPP
#define MATH_HPP

#include <cmath>
#include <glm/glm.hpp>

static glm::vec3 forward(float yaw, float pitch) {
    float sinPitch = sin(pitch);
    float cosPitch = cos(pitch);
    float sinYaw = sin(yaw);
    float cosYaw = cos(yaw);

    return {
            sinPitch * cosYaw,
            -cosPitch,
            sinPitch * sinYaw
    };
}

static glm::vec3 side(float yaw) {
    return forward(yaw + glm::radians(90.0f), glm::radians(90.0f));
}

#endif // MATH_HPP
