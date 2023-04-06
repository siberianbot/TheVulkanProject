#ifndef RESOURCES_RESOURCETYPE_HPP
#define RESOURCES_RESOURCETYPE_HPP

#include <string>

enum ResourceType {
    UNKNOWN_RESOURCE,
    MESH_RESOURCE,
    IMAGE_RESOURCE,
    SHADER_CODE_RESOURCE,
    SHADER_BINARY_RESOURCE
};

ResourceType fromString(const std::string &value);

std::string toString(const ResourceType &value);

#endif // RESOURCES_RESOURCETYPE_HPP
