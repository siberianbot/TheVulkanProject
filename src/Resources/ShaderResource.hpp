#ifndef RESOURCES_SHADERRESOURCE_HPP
#define RESOURCES_SHADERRESOURCE_HPP

#include <memory>

#include "Resource.hpp"

class RenderingObjectsAllocator;
class ShaderObject;

class ShaderResource : public Resource {
private:
    std::shared_ptr<RenderingObjectsAllocator> _renderingObjectsAllocator;

    std::shared_ptr<ShaderObject> _shader = nullptr;
    std::vector<char> _shaderCode;

    std::vector<char> readContent(const std::filesystem::path &path);
    void writeContent(const std::filesystem::path &path, const std::string &content);

public:
    ShaderResource(const std::string &id,
                   const std::filesystem::path &binPath, const std::filesystem::path &codePath,
                   const std::shared_ptr<RenderingObjectsAllocator> &renderingObjectsAllocator);
    ~ShaderResource() override = default;

    void load() override;
    void unload() override;

    void loadCode();
    void saveCode(const std::string &content);
    void unloadCode();

    [[nodiscard]] ResourceType type() const override { return SHADER_RESOURCE; }

    [[nodiscard]] std::shared_ptr<ShaderObject> shader() const { return this->_shader; }
    [[nodiscard]] const std::vector<char> &shaderCode() const { return this->_shaderCode; }

    bool build();
};


#endif // RESOURCES_SHADERRESOURCE_HPP
