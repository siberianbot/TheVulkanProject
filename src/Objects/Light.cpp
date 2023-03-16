#include "Light.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "src/Math/Math.hpp"

Light::Light() {
    //
}

Light::Light(const glm::vec3 &position, const glm::vec3 &color, const float &radius)
        : _position(position),
          _color(color),
          _radius(radius) {
    //
}

glm::mat4 Light::getProjectionMatrix() {
    glm::mat4 projection = glm::mat4(1);

    switch (this->_kind) {
        case POINT_LIGHT:
            projection = glm::perspective(glm::radians(90.0f), 1.0f, this->_near, this->_far);
            break;

        case SPOT_LIGHT:
            projection = glm::perspective(this->_fov, 1.0f, this->_near, this->_far);
            break;

        case RECT_LIGHT:
            projection = glm::ortho(-this->_rect.x, this->_rect.x, -this->_rect.y, this->_rect.y,
                                    this->_near, this->_far);
            break;
    }

    projection[1][1] *= -1;

    return projection;
}

glm::mat4 Light::getViewMatrix() {
    glm::vec3 forward = ::forward(this->_rotation.x, this->_rotation.y);
    glm::vec3 up = glm::vec3(0, 1, 0);

    return glm::lookAt(this->_position, this->_position + forward, up);
}

glm::mat4 Light::getViewMatrix(const glm::vec3 &forward) {
    glm::vec3 up = glm::vec3(0, 1, 0);

    return glm::lookAt(this->_position, this->_position + forward, up);
}
