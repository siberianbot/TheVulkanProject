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
class EventQueue;
class Resource;

class ResourceDatabase {
private:
    std::shared_ptr<Log> _log;
    std::shared_ptr<EventQueue> _eventQueue;

    std::map<ResourceId, std::shared_ptr<Resource>> _resources;

    void addResource(const std::shared_ptr<Resource> &resource);
    std::shared_ptr<Resource> getResource(const ResourceId &id);

    void addDirectory(const std::filesystem::path &path);

    void readResourceEntry(const std::string &prefix, const std::filesystem::path &basePath,
                           const nlohmann::json &entry);
    void tryReadResourceEntry(const std::string &prefix, const std::filesystem::path &basePath,
                              const nlohmann::json &entry);

public:
    ResourceDatabase(const std::shared_ptr<Log> &log,
                     const std::shared_ptr<EventQueue> &eventQueue);

    void clear();

    void tryAddDirectory(const std::filesystem::path &path);

    [[nodiscard]] std::optional<std::weak_ptr<Resource>> tryGetResource(const ResourceId &id);

    [[nodiscard]] const std::map<std::string, std::shared_ptr<Resource>> &resources() const { return this->_resources; }
};

#endif // RESOURCES_RESOURCEDATABASE_HPP
