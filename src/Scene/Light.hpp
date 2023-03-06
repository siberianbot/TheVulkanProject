#ifndef SCENE_LIGHT_HPP
#define SCENE_LIGHT_HPP

#include <glm/vec3.hpp>

class Light {
private:
    glm::vec3 _position;
    glm::vec3 _color;
    float _radius;

public:
    Light(const glm::vec3 &position, const glm::vec3 &color, const float &radius);

    glm::vec3 &position() { return this->_position; }
    glm::vec3 &color() { return this->_color; }
    float &radius() { return this->_radius; }
};

#endif // SCENE_LIGHT_HPP
