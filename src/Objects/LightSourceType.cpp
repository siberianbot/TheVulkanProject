#include "LightSourceType.hpp"

#include <fmt/core.h>

static constexpr const char *POINT_LIGHT_SOURCE_NAME = "point";
static constexpr const char *CONE_LIGHT_SOURCE_NAME = "cone";
static constexpr const char *RECTANGLE_LIGHT_SOURCE_NAME = "rectangle";

LightSourceType lightSourceTypeFromString(const std::string &value) {
    if (value == POINT_LIGHT_SOURCE_NAME) {
        return POINT_LIGHT_SOURCE;
    }

    if (value == CONE_LIGHT_SOURCE_NAME) {
        return CONE_LIGHT_SOURCE;
    }

    if (value == RECTANGLE_LIGHT_SOURCE_NAME) {
        return RECTANGLE_LIGHT_SOURCE;
    }

    throw std::out_of_range(fmt::format("Unknown resource type {0}", value));
}

std::string toString(const LightSourceType &type) {
    switch (type) {
        case POINT_LIGHT_SOURCE:
            return POINT_LIGHT_SOURCE_NAME;

        case CONE_LIGHT_SOURCE:
            return CONE_LIGHT_SOURCE_NAME;

        case RECTANGLE_LIGHT_SOURCE:
            return RECTANGLE_LIGHT_SOURCE_NAME;
    }

    throw std::out_of_range("Unknown resource type");
}
