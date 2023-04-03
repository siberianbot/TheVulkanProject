#ifndef RESOURCES_RESOURCEMANAGER_HPP
#define RESOURCES_RESOURCEMANAGER_HPP

#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>

#include <nlohmann/json.hpp>

#include "src/Types/ResourceType.hpp"

class Log;
class RenderingObjectsAllocator;
class Resource;

class ResourceManager {
private:
    using ResourceEntryReaderFunc = std::function<std::shared_ptr<Resource>(const std::string &id,
                                                                            const std::filesystem::path &path,
                                                                            const nlohmann::json &)>;

    std::shared_ptr<Log> _log;
    std::shared_ptr<RenderingObjectsAllocator> _renderingObjectsAllocator;

    std::map<std::string, std::shared_ptr<Resource>> _resources;
    std::map<ResourceType, ResourceEntryReaderFunc> _resourceEntryReader;

    void addResource(const std::shared_ptr<Resource> &resource);
    std::shared_ptr<Resource> getResource(const std::string &id, ResourceType type);

    void addDirectory(const std::filesystem::path &path);
    std::shared_ptr<Resource> readResourceEntry(const std::filesystem::path &path, const nlohmann::json &entry);

public:
    ResourceManager(const std::shared_ptr<Log> &log,
                    const std::shared_ptr<RenderingObjectsAllocator> &renderingObjectsAllocator);

    void tryAddDirectory(const std::filesystem::path &path);

    void unloadAll();
    void removeAll();

    const std::map<std::string, std::shared_ptr<Resource>> &resources() const { return this->_resources; }

    template<typename T>
    [[nodiscard]] std::optional<std::weak_ptr<T>> tryGetResource(const std::string &id, ResourceType type);
};

class CubeImageResource;
class ImageResource;
class MeshResource;
class ShaderResource;

extern template std::optional<std::weak_ptr<CubeImageResource>> ResourceManager::tryGetResource<CubeImageResource>(
        const std::string &id, ResourceType type);

extern template std::optional<std::weak_ptr<ImageResource>> ResourceManager::tryGetResource<ImageResource>(
        const std::string &id, ResourceType type);

extern template std::optional<std::weak_ptr<MeshResource>> ResourceManager::tryGetResource<MeshResource>(
        const std::string &id, ResourceType type);

extern template std::optional<std::weak_ptr<ShaderResource>> ResourceManager::tryGetResource<ShaderResource>(
        const std::string &id, ResourceType type);

#endif // RESOURCES_RESOURCEMANAGER_HPP
