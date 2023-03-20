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
class RendererAllocator;

class ResourceManager {
private:
    std::shared_ptr<RendererAllocator> _rendererAllocator;

    std::map<std::string, Resource *> _resources;

    Resource *getResource(const std::string &id, ResourceType type);

public:
    ResourceManager(const std::shared_ptr<RendererAllocator> &rendererAllocator);
    ~ResourceManager();

    void addDataDir(const std::filesystem::path &path);

    [[nodiscard]] std::optional<std::string> getIdOf(Resource *resource) const;

    [[nodiscard]] MeshResource *loadMesh(const std::string &id);
    [[nodiscard]] ImageResource *loadImage(const std::string &id);
    [[nodiscard]] CubeImageResource *loadCubeImage(const std::string &id);
    [[nodiscard]] ShaderResource *loadShader(const std::string &id);

    [[nodiscard]] ImageResource *loadDefaultImage();

    void unloadAll();

    const std::map<std::string, Resource *> &resources() const { return this->_resources; }
};

#endif // RESOURCES_RESOURCEMANAGER_HPP
