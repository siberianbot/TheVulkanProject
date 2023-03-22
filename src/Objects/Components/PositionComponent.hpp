#ifndef OBJECTS_COMPONENTS_POSITIONCOMPONENT_HPP
#define OBJECTS_COMPONENTS_POSITIONCOMPONENT_HPP

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

#include "src/Objects/Components/IComponent.hpp"

struct PositionComponent : public IComponent {
public:
    ~PositionComponent() override = default;

    glm::vec3 position = glm::vec3(0);
    glm::vec3 rotation = glm::vec3(0);
    glm::vec3 scale = glm::vec3(1);

    glm::mat3 rotationMat3() const;
    glm::mat4 rotationMat4() const;

    glm::mat4 model() const;
};

#endif // OBJECTS_COMPONENTS_POSITIONCOMPONENT_HPP
