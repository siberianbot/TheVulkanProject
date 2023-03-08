#ifndef SCENE_LIGHT_HPP
#define SCENE_LIGHT_HPP

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>

enum LightKind : uint32_t {
    POINT_LIGHT = 0,
    SPOT_LIGHT = 1,
    RECT_LIGHT = 2
};

class Light {
private:
    LightKind _kind = SPOT_LIGHT;
    glm::vec3 _position;
    glm::vec2 _rotation = glm::vec2(0);
    glm::vec3 _color = glm::vec3(1);
    float _radius = 50.0f;
    float _fov = glm::radians(90.0f);
    float _near = 1.0f;
    float _far = 100.0f;

public:
    explicit Light();
    [[deprecated]] Light(const glm::vec3 &position, const glm::vec3 &color, const float &radius);

    LightKind &kind() { return this->_kind; }
    glm::vec3 &position() { return this->_position; }
    glm::vec2 &rotation() { return this->_rotation; }
    glm::vec3 &color() { return this->_color; }
    float &radius() { return this->_radius; }
    float &fov() { return this->_fov; }
    float &near() { return this->_near; }
    float &far() { return this->_far; }

    glm::mat4 getProjectionMatrix();
    glm::mat4 getViewMatrix();
};

#endif // SCENE_LIGHT_HPP
