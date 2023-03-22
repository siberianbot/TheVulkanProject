#ifndef OBJECTS_CAMERA_HPP
#define OBJECTS_CAMERA_HPP

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "src/Objects/Object.hpp"

struct PositionComponent;

class Camera : public Object {
private:
    std::shared_ptr<PositionComponent> _position;

    float _near;
    float _far;
    float _fov;

public:
    explicit Camera();
    ~Camera() override = default;

    std::string displayName() override;

    [[nodiscard]] float &near() { this->_near; }
    [[nodiscard]] float &far() { this->_far; }
    [[nodiscard]] float &fov() { this->_fov; }

    [[nodiscard]] glm::vec3 forward() const;
    [[nodiscard]] glm::vec3 side() const;
    [[nodiscard]] glm::vec3 up() const;

    [[nodiscard]] glm::mat4 projection(float aspect) const;
    [[nodiscard]] glm::mat4 view(bool ignorePosition) const;
};

#endif // OBJECTS_CAMERA_HPP
