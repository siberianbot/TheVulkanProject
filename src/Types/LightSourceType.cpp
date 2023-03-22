#include "LightSourceType.hpp"

#include <stdexcept>

static constexpr const char *POINT_LIGHT_SOURCE_NAME = "point";
static constexpr const char *CONE_LIGHT_SOURCE_NAME = "cone";
static constexpr const char *RECTANGLE_LIGHT_SOURCE_NAME = "rectangle";

std::string toString(const LightSourceType &type) {
    switch (type) {
        case POINT_LIGHT_SOURCE:
            return POINT_LIGHT_SOURCE_NAME;

        case CONE_LIGHT_SOURCE:
            return CONE_LIGHT_SOURCE_NAME;

        case RECTANGLE_LIGHT_SOURCE:
            return RECTANGLE_LIGHT_SOURCE_NAME;
    }

    throw std::runtime_error("Unknown resource type");
}
