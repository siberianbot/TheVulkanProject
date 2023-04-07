#include "LightSource.hpp"

#include <fmt/core.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "src/Debug/UI/ObjectEditVisitor.hpp"
#include "src/Objects/Components/PositionComponent.hpp"

static constexpr const float NEAR = 0.01;

LightSource::LightSource() : LightSource(std::make_shared<PositionComponent>()) {
    //
}

LightSource::LightSource(const std::shared_ptr<PositionComponent> &position)
        : _position(position),
          _type(POINT_LIGHT_SOURCE),
          _color(glm::vec3(1)),
          _range(10),
          _angle(glm::radians(90.0f)),
          _rect(glm::vec2(5)) {
    this->_components.push_back(this->_position);
}

std::string LightSource::displayName() {
    return fmt::format("({0}) light source", this->id());
}

glm::vec3 LightSource::forward() const {
    return this->_position->rotationMat3() * glm::vec3(1, 0, 0);
}

glm::mat4 LightSource::projection() const {
    glm::mat4 projection = glm::mat4(1);

    switch (this->_type) {
        case POINT_LIGHT_SOURCE:
        case CONE_LIGHT_SOURCE:
            projection = glm::perspective(this->_angle, 1.0f, NEAR, this->_range);
            break;

        case RECTANGLE_LIGHT_SOURCE:
            projection = glm::ortho(-this->_rect.x, this->_rect.x, -this->_rect.y, this->_rect.y,
                                    NEAR, this->_range);
            break;
    }

    return projection;
}

glm::mat4 LightSource::view() const {
    glm::mat3 rotation = this->_position->rotationMat3();
    glm::vec3 forward = rotation * glm::vec3(1, 0, 0);
    glm::vec3 up = rotation * glm::vec3(0, 1, 0);

    return glm::lookAt(this->_position->position(), this->_position->position() + forward, up);
}

glm::mat4 LightSource::view(const glm::vec3 &forward) const {
    return glm::lookAt(this->_position->position(), this->_position->position() + forward, glm::vec3(0, 1, 0));
}

void LightSource::acceptEdit(const std::shared_ptr<ObjectEditVisitor> &visitor) {
    visitor->drawLightSourceObject(this);
}
