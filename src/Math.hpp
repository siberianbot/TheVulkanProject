#ifndef MATH_HPP
#define MATH_HPP

#include <glm/vec3.hpp>

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

#endif // MATH_HPP
