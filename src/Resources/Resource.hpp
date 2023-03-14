#ifndef RESOURCES_RESOURCE_HPP
#define RESOURCES_RESOURCE_HPP

#include <filesystem>
#include <vector>

#include "src/Types/ResourceType.hpp"

class Resource {
protected:
    bool _isLoaded = false;
    std::vector<std::filesystem::path> _paths;

    Resource(const std::vector<std::filesystem::path> &paths);

public:
    virtual ~Resource() = default;

    virtual void load();
    virtual void unload();

    [[nodiscard]] virtual ResourceType type() const = 0;

    [[nodiscard]] bool isLoaded() const { return this->_isLoaded; }
    [[nodiscard]] const std::vector<std::filesystem::path> &paths() const { return this->_paths; }
};

#endif // RESOURCES_RESOURCE_HPP
