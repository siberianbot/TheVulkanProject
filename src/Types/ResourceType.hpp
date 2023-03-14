#ifndef TYPES_RESOURCETYPE_HPP
#define TYPES_RESOURCETYPE_HPP

#include <string>

enum ResourceType {
    MESH_RESOURCE,
    IMAGE_RESOURCE,
    CUBE_IMAGE_RESOURCE,
    SHADER_RESOURCE
};

ResourceType fromString(const std::string &str);

std::string toString(const ResourceType &type);

#endif // TYPES_RESOURCETYPE_HPP
