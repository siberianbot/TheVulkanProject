#include "ResourceType.hpp"

#include <fmt/core.h>

static constexpr const char *UNKNOWN_RESOURCE_STRING = "unknown";
static constexpr const char *MESH_RESOURCE_STRING = "mesh";
static constexpr const char *IMAGE_RESOURCE_STRING = "image";
static constexpr const char *SCENE_RESOURCE_STRING = "scene";
static constexpr const char *SHADER_CODE_RESOURCE_STRING = "shader-code";
static constexpr const char *SHADER_BINARY_RESOURCE_STRING = "shader-binary";

ResourceType resourceTypeFromString(const std::string &value) {
    if (value == MESH_RESOURCE_STRING) {
        return MESH_RESOURCE;
    }

    if (value == IMAGE_RESOURCE_STRING) {
        return IMAGE_RESOURCE;
    }

    if (value == SCENE_RESOURCE_STRING) {
        return SCENE_RESOURCE;
    }

    if (value == SHADER_CODE_RESOURCE_STRING) {
        return SHADER_CODE_RESOURCE;
    }

    if (value == SHADER_BINARY_RESOURCE_STRING) {
        return SHADER_BINARY_RESOURCE;
    }

    return UNKNOWN_RESOURCE;
}

std::string toString(const ResourceType &value) {
    switch (value) {
        case UNKNOWN_RESOURCE:
            return UNKNOWN_RESOURCE_STRING;

        case MESH_RESOURCE:
            return MESH_RESOURCE_STRING;

        case IMAGE_RESOURCE:
            return IMAGE_RESOURCE_STRING;

        case SCENE_RESOURCE:
            return SCENE_RESOURCE_STRING;

        case SHADER_CODE_RESOURCE:
            return SHADER_CODE_RESOURCE_STRING;

        case SHADER_BINARY_RESOURCE:
            return SHADER_BINARY_RESOURCE_STRING;
    }

    throw std::out_of_range("Unknown resource type");
}
