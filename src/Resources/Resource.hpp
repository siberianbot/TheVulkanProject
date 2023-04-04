#ifndef RESOURCES_RESOURCE_HPP
#define RESOURCES_RESOURCE_HPP

#include <filesystem>
#include <string>
#include <vector>

#include "src/Resources/ResourceId.hpp"
#include "src/Resources/ResourceType.hpp"

class Resource {
private:
    ResourceId _id;
    ResourceType _type;
    std::vector<std::filesystem::path> _paths;

public:
    Resource(const ResourceId &id,
             const ResourceType &type,
             const std::vector<std::filesystem::path> &paths);

    [[nodiscard]] const ResourceId &id() const { return this->_id; }

    [[nodiscard]] const ResourceType &type() const { return this->_type; };

    [[nodiscard]] const std::vector<std::filesystem::path> &paths() const { return this->_paths; }
};

#endif // RESOURCES_RESOURCE_HPP
