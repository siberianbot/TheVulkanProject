#include "Camera.hpp"

#include <sstream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "src/Debug/UI/ObjectEditVisitor.hpp"
#include "src/Objects/Components/PositionComponent.hpp"

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
    return this->_position->rotationMat3() * glm::vec3(1, 0, 0);
}

glm::vec3 Camera::side() const {
    return this->_position->rotationMat3() * glm::vec3(0, 0, 1);
}

glm::vec3 Camera::up() const {
    return this->_position->rotationMat3() * glm::vec3(0, 1, 0);
}

glm::mat4 Camera::projection(float aspect) const {
    return glm::perspective(this->_fov, aspect, this->_near, this->_far);
}

glm::mat4 Camera::view(bool ignorePosition) const {
    glm::mat3 rotation = this->_position->rotationMat3();
    glm::vec3 forward = rotation * glm::vec3(1, 0, 0);
    glm::vec3 up = rotation * glm::vec3(0, 1, 0);

    return ignorePosition
           ? glm::lookAt(glm::vec3(0), forward, up)
           : glm::lookAt(this->_position->position, this->_position->position + forward, up);
}

void Camera::acceptEdit(const std::shared_ptr<ObjectEditVisitor> &visitor) {
    visitor->drawCameraObject(this);
}
