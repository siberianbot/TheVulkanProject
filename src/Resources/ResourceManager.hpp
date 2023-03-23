#ifndef RESOURCES_RESOURCEMANAGER_HPP
#define RESOURCES_RESOURCEMANAGER_HPP

#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>

#include "src/Types/ResourceType.hpp"

class Resource;
class CubeImageResource;
class ImageResource;
class MeshResource;
class ShaderResource;
class RenderingObjectsAllocator;

class ResourceManager {
private:
    std::shared_ptr<RenderingObjectsAllocator> _renderingObjectsAllocator;

    std::map<std::string, std::shared_ptr<Resource>> _resources;

    std::shared_ptr<Resource> getResource(const std::string &name, ResourceType type);

public:
    ResourceManager(const std::shared_ptr<RenderingObjectsAllocator> &renderingObjectsAllocator);

    void addDataDir(const std::filesystem::path &path);

    void unloadAll();
    void removeAll();

    [[nodiscard]] std::shared_ptr<MeshResource> loadMesh(const std::string &id);
    [[nodiscard]] std::shared_ptr<ImageResource> loadImage(const std::string &id);
    [[nodiscard]] std::shared_ptr<CubeImageResource> loadCubeImage(const std::string &id);
    [[nodiscard]] std::shared_ptr<ShaderResource> loadShader(const std::string &id);

    [[nodiscard]] std::shared_ptr<ImageResource> loadDefaultImage();

    const std::map<std::string, std::shared_ptr<Resource>> &resources() const { return this->_resources; }
};

#endif // RESOURCES_RESOURCEMANAGER_HPP
