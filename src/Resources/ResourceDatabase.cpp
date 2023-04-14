#include "ResourceDatabase.hpp"

#include <fstream>

#include <fmt/core.h>

#include "src/Engine/EngineError.hpp"
#include "src/Engine/Log.hpp"
#include "src/Events/EventQueue.hpp"
#include "src/Resources/Resource.hpp"

static constexpr const char *RESOURCE_DATABASE_TAG = "ResourceDatabase";
static constexpr const char *RESOURCE_DATABASE_FILE = "resources.json";

static constexpr const char *RESOURCE_ENTRY_ID_TAG = "id";
static constexpr const char *RESOURCE_ENTRY_TYPE_TAG = "type";
static constexpr const char *RESOURCE_ENTRY_PATH_TAG = "path";
static constexpr const char *RESOURCE_ENTRY_ITEMS_TAG = "items";
static constexpr const char *RESOURCE_TYPE_GROUP = "group";

void ResourceDatabase::addResource(const std::shared_ptr<Resource> &resource) {
    ResourceId id = resource->id();
    EventType eventType;

    auto it = this->_resources.find(id);

    if (it != this->_resources.end()) {
        it->second = resource;
        eventType = REPLACED_RESOURCE_EVENT;

        this->_log->info(RESOURCE_DATABASE_TAG, fmt::format("Replaced resource {0}", id));
    } else {
        this->_resources[id] = resource;
        eventType = ADDED_RESOURCE_EVENT;

        this->_log->info(RESOURCE_DATABASE_TAG, fmt::format("Added resource {0}", id));
    }

    this->_eventQueue->pushEvent({.type = eventType, .value = id});
}

std::shared_ptr<Resource> ResourceDatabase::getResource(const ResourceId &id) {
    auto it = this->_resources.find(id);

    if (it == this->_resources.end()) {
        throw EngineError(fmt::format("Resource {0} not found", id));
    }

    return it->second;
}

void ResourceDatabase::addDirectory(const std::filesystem::path &path) {
    this->_log->info(RESOURCE_DATABASE_TAG, fmt::format("Reading resources root {0}", path.string()));

    std::ifstream databaseStream(path / RESOURCE_DATABASE_FILE);

    if (!databaseStream.is_open()) {
        throw EngineError(fmt::format("Failed to load resources root {0}", path.string()));
    }

    this->tryReadResourceEntry("", path, nlohmann::json::parse(databaseStream, nullptr, true, true));
}

void ResourceDatabase::readResourceEntry(const std::string &prefix, const std::filesystem::path &basePath,
                                         const nlohmann::json &entry) {
    if (!entry.contains(RESOURCE_ENTRY_ID_TAG)) {
        throw EngineError("Resource entry does not contain id");
    }

    if (!entry.contains(RESOURCE_ENTRY_TYPE_TAG)) {
        throw EngineError("Resource entry does not contain type");
    }

    std::string id;

    if (prefix.empty()) {
        id = entry[RESOURCE_ENTRY_ID_TAG];
    } else {
        id = fmt::format("{0}/{1}", prefix, entry[RESOURCE_ENTRY_ID_TAG]);
    }

    std::string type = entry[RESOURCE_ENTRY_TYPE_TAG];

    if (type == RESOURCE_TYPE_GROUP) {
        if (!entry.contains(RESOURCE_ENTRY_ITEMS_TAG)) {
            throw EngineError("Resource entry does not contain items");
        }

        for (const nlohmann::json &item: entry[RESOURCE_ENTRY_ITEMS_TAG]) {
            this->tryReadResourceEntry(id, basePath, item);
        }
    } else {
        if (!entry.contains(RESOURCE_ENTRY_PATH_TAG)) {
            throw EngineError("Resource entry does not contain path");
        }

        std::filesystem::path path = basePath / entry[RESOURCE_ENTRY_PATH_TAG];
        std::shared_ptr<Resource> resource = std::make_shared<Resource>(id, fromString<ResourceType>(type), path);

        this->addResource(resource);

        if (resource->type() == UNKNOWN_RESOURCE) {
            this->_log->warning(RESOURCE_DATABASE_TAG, fmt::format("Resource {0} have unknown type {1}", id, type));
        }
    }
}

void ResourceDatabase::tryReadResourceEntry(const std::string &prefix, const std::filesystem::path &basePath,
                                            const nlohmann::json &entry) {
    try {
        this->readResourceEntry(prefix, basePath, entry);
    } catch (const std::exception &error) {
        this->_log->warning(RESOURCE_DATABASE_TAG, error);
    }
}

ResourceDatabase::ResourceDatabase(const std::shared_ptr<Log> &log,
                                   const std::shared_ptr<EventQueue> &eventQueue)
        : _log(log),
          _eventQueue(eventQueue) {
    //
}

void ResourceDatabase::clear() {
    for (const auto &[id, resource]: this->_resources) {
        this->_eventQueue->pushEvent({.type = REMOVED_RESOURCE_EVENT, .value = id});
    }

    this->_resources.clear();
}

void ResourceDatabase::tryAddDirectory(const std::filesystem::path &path) {
    try {
        this->addDirectory(path);
    } catch (const std::exception &error) {
        this->_log->error(RESOURCE_DATABASE_TAG, error);
    }
}

std::optional<std::weak_ptr<Resource>> ResourceDatabase::tryGetResource(const ResourceId &id) {
    try {
        return this->getResource(id);
    } catch (const std::exception &error) {
        this->_log->error(RESOURCE_DATABASE_TAG, error);

        return std::nullopt;
    }
}
