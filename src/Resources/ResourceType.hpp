#ifndef RESOURCES_RESOURCETYPE_HPP
#define RESOURCES_RESOURCETYPE_HPP

#include <stdexcept>
#include <string>
#include <string_view>

#include "src/Utils/FromString.hpp"

static constexpr const std::string_view UNKNOWN_RESOURCE_STRING = "unknown";
static constexpr const std::string_view MESH_RESOURCE_STRING = "mesh";
static constexpr const std::string_view IMAGE_RESOURCE_STRING = "image";
static constexpr const std::string_view SCENE_RESOURCE_STRING = "scene";
static constexpr const std::string_view SHADER_CODE_RESOURCE_STRING = "shader-code";
static constexpr const std::string_view SHADER_BINARY_RESOURCE_STRING = "shader-binary";

enum ResourceType {
    UNKNOWN_RESOURCE,
    MESH_RESOURCE,
    IMAGE_RESOURCE,
    SCENE_RESOURCE,
    SHADER_CODE_RESOURCE,
    SHADER_BINARY_RESOURCE
};

template<>
constexpr ResourceType fromString<ResourceType>(const std::string_view &value) {
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

constexpr std::string_view toString(const ResourceType &value) {
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

#endif // RESOURCES_RESOURCETYPE_HPP
