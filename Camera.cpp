#include "Camera.hpp"

#include <glm/ext/matrix_transform.hpp>
#include "Math.hpp"

Camera::Camera(const glm::vec3 &pos, const float &yaw, const float &pitch)
        : _pos(pos),
          _yaw(yaw),
          _pitch(pitch) {
    //
}

glm::mat4 Camera::view() {
    return glm::lookAt(
            this->_pos,
            this->_pos + forward(this->_yaw, this->_pitch),
            glm::vec3(0, 1, 0));
}

