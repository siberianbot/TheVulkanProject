#ifndef RESOURCES_RESOURCEMANAGER_HPP
#define RESOURCES_RESOURCEMANAGER_HPP

#include <filesystem>
#include <map>
#include <string>
#include <vector>

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

public:
    void addDataDir(const std::filesystem::path &path);

    std::map<std::string, Resource> &resources() { return this->_resources; }
};

#endif // RESOURCES_RESOURCEMANAGER_HPP
