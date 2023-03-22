#ifndef OBJECTS_LIGHTSOURCE_HPP
#define OBJECTS_LIGHTSOURCE_HPP

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "src/Objects/Object.hpp"
#include "src/Types/LightSourceType.hpp"

struct PositionComponent;

class LightSource : public Object {
private:
    std::shared_ptr<PositionComponent> _position;

    LightSourceType _type;
    glm::vec3 _color;
    float _range;
    float _angle;
    glm::vec2 _rect;

public:
    explicit LightSource();
    ~LightSource() override = default;

    std::string displayName() override;

    [[nodiscard]] LightSourceType &type() { return this->_type; }
    [[nodiscard]] glm::vec3 &color() { return this->_color; }
    [[nodiscard]] float &range() { return this->_range; }

    // point and cone light sources only
    [[nodiscard]] float &angle() { return this->_angle; }

    // rectangle light source only
    [[nodiscard]] glm::vec2 &rect() { return this->_rect; }

    [[nodiscard]] glm::vec3 forward() const;
    [[nodiscard]] glm::mat4 projection() const;
    [[nodiscard]] glm::mat4 view() const;
    [[nodiscard]] glm::mat4 view(const glm::vec3 &forward) const;
};


#endif // OBJECTS_LIGHTSOURCE_HPP
