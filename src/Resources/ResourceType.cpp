#include "ResourceType.hpp"

#include <fmt/core.h>

static constexpr const char *MESH_RESOURCE_STRING = "mesh";
static constexpr const char *IMAGE_RESOURCE_STRING = "image";
static constexpr const char *CUBE_IMAGE_RESOURCE_STRING = "cube-image";
static constexpr const char *SHADER_RESOURCE_STRING = "shader";

ResourceType fromString(const std::string &value) {
    if (value == MESH_RESOURCE_STRING) {
        return MESH_RESOURCE;
    }

    if (value == IMAGE_RESOURCE_STRING) {
        return IMAGE_RESOURCE;
    }

    if (value == CUBE_IMAGE_RESOURCE_STRING) {
        return CUBE_IMAGE_RESOURCE;
    }

    if (value == SHADER_RESOURCE_STRING) {
        return SHADER_RESOURCE;
    }

    throw std::out_of_range(fmt::format("Unknown resource type {0}", value));
}

std::string toString(const ResourceType &value) {
    switch (value) {
        case MESH_RESOURCE:
            return MESH_RESOURCE_STRING;

        case IMAGE_RESOURCE:
            return IMAGE_RESOURCE_STRING;

        case CUBE_IMAGE_RESOURCE:
            return CUBE_IMAGE_RESOURCE_STRING;

        case SHADER_RESOURCE:
            return SHADER_RESOURCE_STRING;
    }

    throw std::out_of_range(fmt::format("Unknown resource type {0}", value));
}
