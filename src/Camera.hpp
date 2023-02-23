#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>

class Camera {
private:
    glm::vec3 _pos;
    float _yaw;
    float _pitch;

public:
    Camera(const glm::vec3 &pos, const float &yaw, const float &pitch);

    glm::vec3 &pos() { return this->_pos; }
    glm::vec3 forward();
    float &yaw() { return this->_yaw; }
    float &pitch() { return this->_pitch; }

    glm::mat4 view();
};

#endif // CAMERA_HPP
