#include "Camera.hpp"

#include <sstream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "src/Debug/UI/ObjectEditVisitor.hpp"
#include "src/Objects/Components/PositionComponent.hpp"

static glm::vec3 radius(float phi, float theta) {
    return glm::vec3(
            sin(phi) * cos(theta),
            cos(phi),
            sin(phi) * sin(theta)
    );
}

Camera::Camera()
        : _position(std::make_shared<PositionComponent>()),
          _near(0.01f),
          _far(100.0f),
          _fov(glm::radians(90.0f)) {
    this->_components.push_back(this->_position);
}

std::string Camera::displayName() {
    std::stringstream ss;
    ss << "(" << this->id() << ") camera";

    return ss.str();
}

glm::vec3 Camera::forward() const {
    float phi = -this->_position->rotation().y;
    float theta = this->_position->rotation().x;

    return radius(phi, theta);
}

glm::vec3 Camera::side() const {
    return glm::cross(this->forward(), this->up());
}

glm::vec3 Camera::up() const {
    float phi = -this->_position->rotation().y - glm::radians(90.0f);
    float theta = this->_position->rotation().x;

    return radius(phi, theta);
}

glm::mat4 Camera::projection(float aspect) const {
    return glm::perspective(this->_fov, aspect, this->_near, this->_far);
}

glm::mat4 Camera::view(bool ignorePosition) const {
    glm::vec3 forward = this->forward();
    glm::vec3 up = this->up();

    return ignorePosition
           ? glm::lookAt(glm::vec3(0), forward, up)
           : glm::lookAt(this->_position->position(), this->_position->position() + forward, up);
}

void Camera::acceptEdit(const std::shared_ptr<ObjectEditVisitor> &visitor) {
    visitor->drawCameraObject(this);
}
