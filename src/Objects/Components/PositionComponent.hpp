#ifndef OBJECTS_COMPONENTS_POSITIONCOMPONENT_HPP
#define OBJECTS_COMPONENTS_POSITIONCOMPONENT_HPP

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

#include "src/Objects/Components/Component.hpp"

class PositionComponent : public Component {
private:
    glm::vec3 _position = glm::vec3(0);
    glm::vec3 _rotation = glm::vec3(0);
    glm::vec3 _scale = glm::vec3(1);

public:
    ~PositionComponent() override = default;

    [[nodiscard]] glm::vec3 &position() { return this->_position; }
    [[nodiscard]] glm::vec3 &rotation() { return this->_rotation; }
    [[nodiscard]] glm::vec3 &scale() { return this->_scale; }

    glm::mat3 rotationMat3() const;
    glm::mat4 rotationMat4() const;

    [[nodiscard]] glm::mat4 model() const;

    void acceptEdit(const std::shared_ptr<ObjectEditVisitor> &visitor) override;
};

#endif // OBJECTS_COMPONENTS_POSITIONCOMPONENT_HPP
