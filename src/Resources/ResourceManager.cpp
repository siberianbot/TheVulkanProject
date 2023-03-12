#include "ResourceManager.hpp"

#include <fstream>

#include <nlohmann/json.hpp>

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

void ResourceManager::addDataDir(const std::filesystem::path &path) {
    std::ifstream manifestStream(path / DATA_DIR_MANIFEST_NAME);

    if (!manifestStream.is_open()) {
        throw std::runtime_error("Failed to load manifestStream.manifest");
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
