#include "ResourceManager.hpp"

#include <fstream>
#include <type_traits>

#include <fmt/core.h>

#include "src/Engine/EngineError.hpp"
#include "src/Engine/Log.hpp"
#include "src/Resources/CubeImageResource.hpp"
#include "src/Resources/ImageResource.hpp"
#include "src/Resources/MeshResource.hpp"
#include "src/Resources/ShaderResource.hpp"

static constexpr const char *RESOURCE_MANAGER_TAG = "ResourceManager";
static constexpr const char *RESOURCE_DIRECTORY_MANIFEST_NAME = "resources.json";

void ResourceManager::addResource(const std::shared_ptr<Resource> &resource) {
    auto it = this->_resources.find(resource->id());

    if (it != this->_resources.end()) {
        it->second->unload();
    }

    this->_resources[resource->id()] = resource;
}

std::shared_ptr<Resource> ResourceManager::getResource(const std::string &id, ResourceType type) {
    if (!this->_resources.contains(id)) {
        throw EngineError(fmt::format("Resource {0} not found", id));
    }

    std::shared_ptr<Resource> resource = this->_resources[id];

    if (resource->type() != type) {
        throw EngineError(fmt::format("Resource {0} type mismatch (expected = {1}, actual = {2})", id, toString(type),
                                      toString(resource->type())));
    }

    return resource;
}

void ResourceManager::addDirectory(const std::filesystem::path &path) {
    this->_log->info(RESOURCE_MANAGER_TAG, fmt::format("Reading resources manifest at {0}", path.string()));

    std::ifstream manifestStream(path / RESOURCE_DIRECTORY_MANIFEST_NAME);

    if (!manifestStream.is_open()) {
        throw EngineError(fmt::format("Failed to load resources manifest at {0}", path.string()));
    }

    nlohmann::json manifest = nlohmann::json::parse(manifestStream);

    for (const nlohmann::json &entry: manifest) {
        try {
            this->addResource(this->readResourceEntry(path, entry));
        } catch (const std::exception &error) {
            this->_log->warning(RESOURCE_MANAGER_TAG, error);
        }
    }
}

std::shared_ptr<Resource> ResourceManager::readResourceEntry(const std::filesystem::path &path,
                                                             const nlohmann::json &entry) {
    if (!entry.contains("id") || !entry.contains("type")) {
        throw EngineError("Resource entry in manifest does not contain id nor type");
    }

    std::string id = entry["id"];
    std::string typeStr = entry["type"];
    ResourceType type = fromString(typeStr);

    auto it = this->_resourceEntryReader.find(type);

    if (it == this->_resourceEntryReader.end()) {
        throw EngineError(fmt::format("Unable to process resource entry {0} with type {1}", id, typeStr));
    }

    return it->second(id, path, entry);
}

ResourceManager::ResourceManager(const std::shared_ptr<Log> &log,
                                 const std::shared_ptr<RenderingObjectsAllocator> &renderingObjectsAllocator)
        : _log(log),
          _renderingObjectsAllocator(renderingObjectsAllocator) {
    this->_resourceEntryReader[MESH_RESOURCE] = [this](const std::string &id, const std::filesystem::path &path,
                                                       const nlohmann::json &entry) {
        return std::make_shared<MeshResource>(id, path / entry["path"],
                                              this->_renderingObjectsAllocator);
    };

    this->_resourceEntryReader[IMAGE_RESOURCE] = [this](const std::string &id, const std::filesystem::path &path,
                                                        const nlohmann::json &entry) {
        return std::make_shared<ImageResource>(id, path / entry["path"],
                                               this->_renderingObjectsAllocator);
    };

    this->_resourceEntryReader[CUBE_IMAGE_RESOURCE] = [this](const std::string &id, const std::filesystem::path &path,
                                                             const nlohmann::json &entry) {
        nlohmann::json pathsJson = entry["paths"];
        uint32_t size = pathsJson.size();

        if (size != 6) {
            throw EngineError(fmt::format("Resource entry {0} for cube-image should contain 6 paths", id));
        }

        std::array<std::filesystem::path, 6> paths;
        for (uint32_t idx = 0; idx < size; idx++) {
            paths[idx] = path / pathsJson[idx];
        }

        return std::make_shared<CubeImageResource>(id, paths, this->_renderingObjectsAllocator);
    };

    this->_resourceEntryReader[SHADER_RESOURCE] = [this](const std::string &id, const std::filesystem::path &path,
                                                         const nlohmann::json &entry) {
        return std::make_shared<ShaderResource>(id, path / entry["bin-path"], path / entry["code-path"],
                                                this->_renderingObjectsAllocator);
    };
}

void ResourceManager::tryAddDirectory(const std::filesystem::path &path) {
    try {
        this->addDirectory(path);
    } catch (const std::exception &error) {
        this->_log->error(RESOURCE_MANAGER_TAG, error);
    }
}

void ResourceManager::unloadAll() {
    for (const auto &[id, resource]: this->_resources) {
        resource->unload();
    }
}

void ResourceManager::removeAll() {
    this->unloadAll();
    this->_resources.clear();
}

template<typename T>
std::optional<std::weak_ptr<T>> ResourceManager::tryGetResource(const std::string &id, ResourceType type) {
    static_assert(std::is_base_of<Resource, T>::value);

    try {
        return std::dynamic_pointer_cast<T>(this->getResource(id, type));
    } catch (const std::exception &error) {
        this->_log->error(RESOURCE_MANAGER_TAG, error);
        return std::nullopt;
    }
}

template std::optional<std::weak_ptr<CubeImageResource>> ResourceManager::tryGetResource<CubeImageResource>(
        const std::string &id, ResourceType type);

template std::optional<std::weak_ptr<ImageResource>> ResourceManager::tryGetResource<ImageResource>(
        const std::string &id, ResourceType type);

template std::optional<std::weak_ptr<MeshResource>> ResourceManager::tryGetResource<MeshResource>(
        const std::string &id, ResourceType type);

template std::optional<std::weak_ptr<ShaderResource>> ResourceManager::tryGetResource<ShaderResource>(
        const std::string &id, ResourceType type);