#include "Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

glm::vec3 forward(float yaw, float pitch) {
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

glm::vec3 Camera::getForwardVector() {
    return forward(this->_yaw, this->_pitch);
}

glm::vec3 Camera::getSideVector() {
    return forward(this->_yaw + glm::radians(90.0f), glm::radians(90.0f));
}

glm::mat4 Camera::getProjectionMatrix(uint32_t width, uint32_t height) {
    return glm::perspective(this->_fov, width / (float) height, this->_near, this->_far);
}

glm::mat4 Camera::getViewMatrix(bool ignorePosition) {
    glm::vec3 forward = this->getForwardVector();
    glm::vec3 up = glm::vec3(0, 1, 0);
    
    if (ignorePosition) {
        return glm::lookAt(glm::vec3(0), forward, up);
    } else {
        return glm::lookAt(this->_position, this->_position + forward, up);
    }
}
