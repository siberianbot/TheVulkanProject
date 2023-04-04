#include "ResourceDatabase.hpp"

#include <fstream>

#include <fmt/core.h>

#include "src/Engine/EngineError.hpp"
#include "src/Engine/Log.hpp"
#include "src/Resources/Resource.hpp"

static constexpr const char *RESOURCE_DATABASE_TAG = "ResourceDatabase";
static constexpr const char *RESOURCE_DATABASE_FILE = "resources.json";

static constexpr const char *RESOURCE_ID_TAG = "id";
static constexpr const char *RESOURCE_TYPE_TAG = "type";
static constexpr const char *RESOURCE_PATH_TAG = "path";
static constexpr const char *RESOURCE_PATHS_TAG = "paths";
static constexpr const char *RESOURCE_BIN_PATH_TAG = "bin-path";
static constexpr const char *RESOURCE_CODE_PATH_TAG = "code-path";

std::shared_ptr<Resource> ResourceDatabase::getResource(const ResourceId &id) {
    auto it = this->_resources.find(id);

    if (it == this->_resources.end()) {
        throw EngineError(fmt::format("Resource {0} not found", id));
    }

    return it->second;
}

void ResourceDatabase::addDirectory(const std::filesystem::path &path) {
    this->_log->info(RESOURCE_DATABASE_TAG, fmt::format("Reading resources database {0}", path.string()));

    std::ifstream manifestStream(path / RESOURCE_DATABASE_FILE);

    if (!manifestStream.is_open()) {
        throw EngineError(fmt::format("Failed to load resources database {0}", path.string()));
    }

    nlohmann::json manifest = nlohmann::json::parse(manifestStream);

    for (const nlohmann::json &entry: manifest) {
        try {
            std::shared_ptr<Resource> resource = this->readResourceEntry(path, entry);
            this->_resources[resource->id()] = resource;
        } catch (const std::exception &error) {
            this->_log->warning(RESOURCE_DATABASE_TAG, error);
        }
    }
}

std::shared_ptr<Resource> ResourceDatabase::readResourceEntry(const std::filesystem::path &basePath,
                                                              const nlohmann::json &entry) {
    if (!entry.contains(RESOURCE_ID_TAG)) {
        throw EngineError("Resource entry does not contain id");
    }

    if (!entry.contains(RESOURCE_TYPE_TAG)) {
        throw EngineError("Resource entry does not contain type");
    }

    std::string id = entry[RESOURCE_ID_TAG];
    std::string type = entry[RESOURCE_TYPE_TAG];

    auto it = this->_resourceEntryReaders.find(type);

    if (it == this->_resourceEntryReaders.end()) {
        throw EngineError(fmt::format("Unable to process resource entry {0} with type {1}", id, type));
    }

    return it->second(id, basePath, entry);
}

ResourceDatabase::ResourceDatabase(const std::shared_ptr<Log> &log)
        : _log(log) {
    this->_resourceEntryReaders[toString(MESH_RESOURCE)] = [](const ResourceId &id,
                                                              const std::filesystem::path &basePath,
                                                              const nlohmann::json &entry) -> std::shared_ptr<Resource> {
        if (!entry.contains(RESOURCE_PATH_TAG)) {
            throw EngineError(fmt::format("Resource entry {0} does not contain path", id));
        }

        std::filesystem::path path = basePath / entry[RESOURCE_PATH_TAG];

        return std::make_shared<Resource>(id, MESH_RESOURCE, std::vector{path});
    };

    this->_resourceEntryReaders[toString(IMAGE_RESOURCE)] = [](const ResourceId &id,
                                                               const std::filesystem::path &basePath,
                                                               const nlohmann::json &entry) -> std::shared_ptr<Resource> {
        if (!entry.contains(RESOURCE_PATH_TAG)) {
            throw EngineError(fmt::format("Resource entry {0} does not contain path", id));
        }

        std::filesystem::path path = basePath / entry[RESOURCE_PATH_TAG];

        return std::make_shared<Resource>(id, IMAGE_RESOURCE, std::vector{path});
    };

    this->_resourceEntryReaders[toString(CUBE_IMAGE_RESOURCE)] = [](const ResourceId &id,
                                                                    const std::filesystem::path &basePath,
                                                                    const nlohmann::json &entry) -> std::shared_ptr<Resource> {
        if (!entry.contains(RESOURCE_PATHS_TAG)) {
            throw EngineError(fmt::format("Resource entry {0} does not contain path collection", id));
        }

        nlohmann::json pathsJson = entry[RESOURCE_PATHS_TAG];
        if (pathsJson.size() != 6) {
            throw EngineError(fmt::format("Resource entry {0} for cube-image should contain 6 paths", id));
        }

        std::array<std::filesystem::path, 6> paths;
        for (uint32_t idx = 0; idx < 6; idx++) {
            paths[idx] = basePath / pathsJson[idx];
        }

        return std::make_shared<Resource>(id, CUBE_IMAGE_RESOURCE, std::vector(paths.begin(), paths.end()));
    };

    this->_resourceEntryReaders[toString(SHADER_RESOURCE)] = [](const ResourceId &id,
                                                                const std::filesystem::path &basePath,
                                                                const nlohmann::json &entry) -> std::shared_ptr<Resource> {
        if (!entry.contains(RESOURCE_BIN_PATH_TAG)) {
            throw EngineError(fmt::format("Resource entry {0} does not contain path to binary", id));
        }

        std::vector<std::filesystem::path> paths = {
                basePath / entry[RESOURCE_BIN_PATH_TAG]
        };

        if (entry.contains(RESOURCE_CODE_PATH_TAG)) {
            paths.push_back(basePath / entry[RESOURCE_CODE_PATH_TAG]);
        }

        return std::make_shared<Resource>(id, SHADER_RESOURCE, paths);
    };
}

void ResourceDatabase::clear() {
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
