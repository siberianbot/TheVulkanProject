#ifndef RESOURCES_RESOURCE_HPP
#define RESOURCES_RESOURCE_HPP

#include <filesystem>
#include <string>
#include <vector>

#include "src/Types/ResourceType.hpp"

class Resource {
protected:
    std::string _id;
    std::vector<std::filesystem::path> _paths;

    bool _isLoaded = false;

    Resource(const std::string &id, const std::vector<std::filesystem::path> &paths);

public:
    virtual ~Resource() = default;

    virtual void load();
    virtual void unload();

    [[nodiscard]] virtual ResourceType type() const = 0;

    [[nodiscard]] const std::string &id() const { return this->_id; }
    [[nodiscard]] const std::vector<std::filesystem::path> &paths() const { return this->_paths; }

    [[nodiscard]] bool isLoaded() const { return this->_isLoaded; }
};

#endif // RESOURCES_RESOURCE_HPP
