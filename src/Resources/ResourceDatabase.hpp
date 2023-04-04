#ifndef RESOURCES_RESOURCEDATABASE_HPP
#define RESOURCES_RESOURCEDATABASE_HPP

#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>

#include <nlohmann/json.hpp>

#include "src/Resources/ResourceId.hpp"

class Log;
class Resource;

class ResourceDatabase {
private:
    using ResourceEntryReader = std::function<std::shared_ptr<Resource>(const ResourceId &,
                                                                        const std::filesystem::path &,
                                                                        const nlohmann::json &)>;

    std::shared_ptr<Log> _log;

    std::map<ResourceId, std::shared_ptr<Resource>> _resources;
    std::map<std::string, ResourceEntryReader> _resourceEntryReaders;

    std::shared_ptr<Resource> getResource(const ResourceId &id);

    void addDirectory(const std::filesystem::path &path);
    std::shared_ptr<Resource> readResourceEntry(const std::filesystem::path &basePath, const nlohmann::json &entry);

public:
    ResourceDatabase(const std::shared_ptr<Log> &log);

    void clear();

    void tryAddDirectory(const std::filesystem::path &path);

    [[nodiscard]] std::optional<std::weak_ptr<Resource>> tryGetResource(const ResourceId &id);

    [[nodiscard]] const std::map<std::string, std::shared_ptr<Resource>> &resources() const { return this->_resources; }
};

#endif // RESOURCES_RESOURCEDATABASE_HPP
