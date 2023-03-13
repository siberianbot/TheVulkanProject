#ifndef OBJECTS_CAMERA_HPP
#define OBJECTS_CAMERA_HPP

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>

class Camera {
private:
    glm::vec3 _position = glm::vec3(0);
    float _yaw = 0;
    float _pitch = 0;
    float _near = 0.1f;
    float _far = 100.0f;
    float _fov = glm::radians(90.0f);

public:
    [[nodiscard]] glm::vec3 &position() { return this->_position; }
    [[nodiscard]] float &yaw() { return this->_yaw; }
    [[nodiscard]] float &pitch() { return this->_pitch; }
    [[nodiscard]] float &near() { return this->_near; }
    [[nodiscard]] float &far() { return this->_far; }
    [[nodiscard]] float &fov() { return this->_fov; }

    glm::vec3 getForwardVector();
    glm::vec3 getSideVector();
    glm::mat4 getProjectionMatrix(uint32_t width, uint32_t height);
    glm::mat4 getViewMatrix(bool ignorePosition);
};

#endif // OBJECTS_CAMERA_HPP
