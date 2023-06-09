#ifndef OBJECTS_CAMERA_HPP
#define OBJECTS_CAMERA_HPP

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "src/Objects/Object.hpp"

class PositionComponent;

class Camera : public Object {
private:
    std::shared_ptr<PositionComponent> _position;

    float _near;
    float _far;
    float _fov;

public:
    explicit Camera();
    explicit Camera(const std::shared_ptr<PositionComponent> &position);

    ~Camera() override = default;

    std::string displayName() override;

    [[nodiscard]] float &near() { return this->_near; }

    [[nodiscard]] float &far() { return this->_far; }

    [[nodiscard]] float &fov() { return this->_fov; }

    [[nodiscard]] glm::vec3 forward() const;
    [[nodiscard]] glm::vec3 side() const;
    [[nodiscard]] glm::vec3 up() const;

    [[nodiscard]] glm::mat4 projection(float aspect) const;
    [[nodiscard]] glm::mat4 view(bool ignorePosition) const;

    [[nodiscard]] const std::shared_ptr<PositionComponent> position() const { return this->_position; }

    void acceptEdit(const std::shared_ptr<ObjectEditVisitor> &visitor) override;
};

#endif // OBJECTS_CAMERA_HPP
