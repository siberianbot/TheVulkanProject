#ifndef TYPES_LIGHTSOURCETYPE_HPP
#define TYPES_LIGHTSOURCETYPE_HPP

#include <string>

enum LightSourceType : uint8_t {
    POINT_LIGHT_SOURCE,
    CONE_LIGHT_SOURCE,
    RECTANGLE_LIGHT_SOURCE
};

std::string toString(const LightSourceType &type);

#endif // TYPES_LIGHTSOURCETYPE_HPP
