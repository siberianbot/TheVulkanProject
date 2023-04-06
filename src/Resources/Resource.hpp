#ifndef RESOURCES_RESOURCE_HPP
#define RESOURCES_RESOURCE_HPP

#include <filesystem>

#include "src/Resources/ResourceId.hpp"
#include "src/Resources/ResourceType.hpp"

class Resource {
private:
    ResourceId _id;
    ResourceType _type;
    std::filesystem::path _path;

public:
    Resource(const ResourceId &id,
             const ResourceType &type,
             const std::filesystem::path &path);

    [[nodiscard]] const ResourceId &id() const { return this->_id; }

    [[nodiscard]] const ResourceType &type() const { return this->_type; };

    [[nodiscard]] const std::filesystem::path &path() const { return this->_path; }
};

#endif // RESOURCES_RESOURCE_HPP
