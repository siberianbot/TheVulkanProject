#include "PositionComponent.hpp"

#include "src/Debug/UI/ObjectEditVisitor.hpp"

glm::mat3 PositionComponent::rotationMat3() const {
    return mat3_cast(glm::quat(this->rotation));
}

glm::mat4 PositionComponent::rotationMat4() const {
    return mat4_cast(glm::quat(this->rotation));
}

glm::mat4 PositionComponent::model() const {
    return glm::translate(glm::mat4(1), this->position) *
           this->rotationMat4() *
           glm::scale(glm::mat4(1), this->scale);
}

void PositionComponent::acceptEdit(const std::shared_ptr<ObjectEditVisitor> &visitor) {
    visitor->drawPositionComponent(this);
}

