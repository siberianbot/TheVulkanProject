#include "ResourceType.hpp"

#include <stdexcept>

static constexpr const char *MESH_RESOURCE_STRING = "mesh";
static constexpr const char *IMAGE_RESOURCE_STRING = "image";
static constexpr const char *CUBE_IMAGE_RESOURCE_STRING = "cube-image";
static constexpr const char *SHADER_RESOURCE_STRING = "shader";

ResourceType fromString(const std::string &str) {
    if (str == MESH_RESOURCE_STRING) {
        return MESH_RESOURCE;
    }

    if (str == IMAGE_RESOURCE_STRING) {
        return IMAGE_RESOURCE;
    }

    if (str == CUBE_IMAGE_RESOURCE_STRING) {
        return CUBE_IMAGE_RESOURCE;
    }

    if (str == SHADER_RESOURCE_STRING) {
        return SHADER_RESOURCE;
    }

    throw std::runtime_error("Unknown resource type");
}

std::string toString(const ResourceType &type) {
    switch (type) {
        case MESH_RESOURCE:
            return MESH_RESOURCE_STRING;

        case IMAGE_RESOURCE:
            return IMAGE_RESOURCE_STRING;

        case CUBE_IMAGE_RESOURCE:
            return CUBE_IMAGE_RESOURCE_STRING;

        case SHADER_RESOURCE:
            return SHADER_RESOURCE_STRING;
    }

    throw std::runtime_error("Unknown resource type");
}
