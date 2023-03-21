#include "ResourceManager.hpp"

#include <fstream>

#include <nlohmann/json.hpp>

#include "CubeImageResource.hpp"
#include "ImageResource.hpp"
#include "MeshResource.hpp"
#include "ShaderResource.hpp"

static constexpr const char *DATA_DIR_MANIFEST_NAME = "resources.json";
static constexpr const char *DEFAULT_IMAGE_NAME = "default_texture";

std::shared_ptr<Resource> ResourceManager::getResource(const std::string &id, ResourceType type) {
    if (!this->_resources.contains(id)) {
        throw std::runtime_error("Resource not found");
    }

    std::shared_ptr<Resource> resource = this->_resources[id];

    if (resource->type() != type) {
        throw std::runtime_error("Invalid resource type");
    }

    return resource;
}

ResourceManager::ResourceManager(const std::shared_ptr<RendererAllocator> &rendererAllocator)
        : _rendererAllocator(rendererAllocator) {
    //
}

void ResourceManager::addDataDir(const std::filesystem::path &path) {
    std::ifstream manifestStream(path / DATA_DIR_MANIFEST_NAME);

    if (!manifestStream.is_open()) {
        throw std::runtime_error("Failed to load resources manifest");
    }

    nlohmann::json manifest = nlohmann::json::parse(manifestStream);

    for (const nlohmann::json &entry: manifest) {
        std::string id = entry["id"];
        ResourceType type = fromString(entry["type"]);
        std::shared_ptr<Resource> resource;

        switch (type) {
            case MESH_RESOURCE: {
                resource = std::make_shared<MeshResource>(path / entry["path"], this->_rendererAllocator);
                break;
            }

            case IMAGE_RESOURCE: {
                resource = std::make_shared<ImageResource>(path / entry["path"], this->_rendererAllocator);
                break;
            }

            case CUBE_IMAGE_RESOURCE: {
                nlohmann::json pathsJson = entry["paths"];
                uint32_t size = pathsJson.size();

                if (size != 6) {
                    throw std::runtime_error("cube-image should contain 6 paths");
                }

                std::array<std::filesystem::path, 6> paths;
                for (uint32_t idx = 0; idx < size; idx++) {
                    paths[idx] = path / pathsJson[idx];
                }

                resource = std::make_shared<CubeImageResource>(paths, this->_rendererAllocator);
                break;
            }

            case SHADER_RESOURCE: {
                resource = std::make_shared<ShaderResource>(path / entry["bin-path"], path / entry["code-path"],
                                                            this->_rendererAllocator);
                break;
            }

            default:
                throw std::runtime_error("Unsupported resource type");
        }

        this->_resources[id] = resource;
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

std::optional<std::string> ResourceManager::getIdOf(const std::shared_ptr<Resource> &resource) const {
    if (resource == nullptr) {
        return std::nullopt;
    }

    auto it = std::find_if(this->_resources.begin(), this->_resources.end(),
                           [&](const auto &pair) {
                               return pair.second == resource;
                           });

    return it != this->_resources.end()
           ? std::make_optional(it->first)
           : std::nullopt;
}

std::shared_ptr<MeshResource> ResourceManager::loadMesh(const std::string &id) {
    std::shared_ptr<MeshResource> resource = std::dynamic_pointer_cast<MeshResource>(
            this->getResource(id, MESH_RESOURCE));

    if (resource == nullptr) {
        throw std::runtime_error("Resource is not MeshResource");
    }

    resource->load();

    return resource;
}

std::shared_ptr<ImageResource> ResourceManager::loadImage(const std::string &id) {
    std::shared_ptr<ImageResource> resource = std::dynamic_pointer_cast<ImageResource>(
            this->getResource(id, IMAGE_RESOURCE));

    if (resource == nullptr) {
        throw std::runtime_error("Resource is not ImageResource");
    }

    resource->load();

    return resource;
}

std::shared_ptr<CubeImageResource> ResourceManager::loadCubeImage(const std::string &id) {
    std::shared_ptr<CubeImageResource> resource = std::dynamic_pointer_cast<CubeImageResource>(
            this->getResource(id, CUBE_IMAGE_RESOURCE));

    if (resource == nullptr) {
        throw std::runtime_error("Resource is not CubeImageResource");
    }

    resource->load();

    return resource;
}

std::shared_ptr<ShaderResource> ResourceManager::loadShader(const std::string &id) {
    std::shared_ptr<ShaderResource> resource = std::dynamic_pointer_cast<ShaderResource>(
            this->getResource(id, SHADER_RESOURCE));

    if (resource == nullptr) {
        throw std::runtime_error("Resource is not ShaderResource");
    }

    resource->load();

    return resource;
}

std::shared_ptr<ImageResource> ResourceManager::loadDefaultImage() {
    std::shared_ptr<ImageResource> resource = std::dynamic_pointer_cast<ImageResource>(
            this->getResource(DEFAULT_IMAGE_NAME, IMAGE_RESOURCE));

    if (resource == nullptr) {
        throw std::runtime_error("Resource is not ImageResource");
    }

    resource->load();

    return resource;
}
