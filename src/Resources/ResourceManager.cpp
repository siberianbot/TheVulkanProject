#include "ResourceManager.hpp"

#include <fstream>

#include <nlohmann/json.hpp>

#include "Mesh.hpp"
#include "Texture.hpp"

static constexpr const char *DATA_DIR_MANIFEST_NAME = "resources.json";

ResourceType fromString(const std::string &str) {
    if (str == "model-obj") {
        return MODEL_OBJ_RESOURCE;
    }

    if (str == "shader-code") {
        return SHADER_CODE_RESOURCE;
    }

    if (str == "shader-spv") {
        return SHADER_SPV_RESOURCE;
    }

    if (str == "image") {
        return IMAGE_RESOURCE;
    }

    throw std::runtime_error("Unknown resource type");
}

Resource ResourceManager::getResource(const std::string &id, ResourceType type) {
    if (!this->_resources.contains(id)) {
        throw std::runtime_error("Resource not found");
    }

    Resource resource = this->_resources[id];

    if (resource.type != type) {
        throw std::runtime_error("Invalid resource type");
    }

    return resource;
}

std::vector<char> ResourceManager::readFile(const std::filesystem::path &path) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to read binary file");
    }

    size_t size = file.tellg();
    std::vector<char> buffer(size);

    file.seekg(0);
    file.read(buffer.data(), size);
    file.close();

    return buffer;
}

void ResourceManager::addDataDir(const std::filesystem::path &path) {
    std::ifstream manifestStream(path / DATA_DIR_MANIFEST_NAME);

    if (!manifestStream.is_open()) {
        throw std::runtime_error("Failed to load resources.json");
    }

    nlohmann::json manifest = nlohmann::json::parse(manifestStream);

    for (const nlohmann::json &entry: manifest) {
        std::string id = entry["id"];

        Resource resource = {
                .type = fromString(entry["type"]),
                .path = path / entry["path"]
        };

        this->_resources[id] = resource;
    }
}

Mesh *ResourceManager::openMesh(const std::string &id) {
    return Mesh::fromFile(this->getResource(id, MODEL_OBJ_RESOURCE).path);
}

Texture *ResourceManager::openTexture(const std::string &id) {
    return Texture::fromFile(this->getResource(id, IMAGE_RESOURCE).path);
}

std::vector<char> ResourceManager::readShaderBinary(const std::string &id) {
    return this->readFile(this->getResource(id, SHADER_SPV_RESOURCE).path);
}

std::vector<char> ResourceManager::readShaderCode(const std::string &id) {
    return this->readFile(this->getResource(id, SHADER_CODE_RESOURCE).path);
}
