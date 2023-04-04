#ifndef RESOURCES_RESOURCETYPE_HPP
#define RESOURCES_RESOURCETYPE_HPP

#include <string>

enum ResourceType {
    MESH_RESOURCE,
    IMAGE_RESOURCE,
    CUBE_IMAGE_RESOURCE,
    SHADER_RESOURCE
};

ResourceType fromString(const std::string &value);

std::string toString(const ResourceType &value);

#endif // RESOURCES_RESOURCETYPE_HPP
