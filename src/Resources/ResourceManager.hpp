#ifndef RESOURCES_RESOURCEMANAGER_HPP
#define RESOURCES_RESOURCEMANAGER_HPP

#include <filesystem>
#include <map>
#include <string>
#include <vector>

class Mesh;
class Texture;

enum ResourceType {
    MODEL_OBJ_RESOURCE,
    SHADER_CODE_RESOURCE,
    SHADER_SPV_RESOURCE,
    IMAGE_RESOURCE
};

struct Resource {
    ResourceType type;
    std::filesystem::path path;
};

class ResourceManager {
private:
    std::map<std::string, Resource> _resources;

    Resource getResource(const std::string &id, ResourceType type);
    std::vector<char> readFile(const std::filesystem::path &path);

public:
    void addDataDir(const std::filesystem::path &path);

    // TODO: rename to represent resource types
    Mesh *openMesh(const std::string &id);
    Texture *openTexture(const std::string &id);

    std::vector<char> readShaderBinary(const std::string &id);
    std::vector<char> readShaderCode(const std::string &id);

    std::map<std::string, Resource> &resources() { return this->_resources; }
};

#endif // RESOURCES_RESOURCEMANAGER_HPP
