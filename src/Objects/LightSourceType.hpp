#ifndef OBJECTS_LIGHTSOURCETYPE_HPP
#define OBJECTS_LIGHTSOURCETYPE_HPP

#include <string>

enum LightSourceType : uint8_t {
    POINT_LIGHT_SOURCE,
    CONE_LIGHT_SOURCE,
    RECTANGLE_LIGHT_SOURCE
};

LightSourceType lightSourceTypeFromString(const std::string &value);

std::string toString(const LightSourceType &type);

#endif // OBJECTS_LIGHTSOURCETYPE_HPP
