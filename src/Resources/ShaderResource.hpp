#ifndef RESOURCES_SHADERRESOURCE_HPP
#define RESOURCES_SHADERRESOURCE_HPP

#include "Resource.hpp"

class RendererAllocator;
class ShaderObject;

class ShaderResource : public Resource {
private:
    RendererAllocator *_rendererAllocator;

    ShaderObject *_shader = nullptr;
    std::vector<char> _shaderCode;

    std::vector<char> readContent(const std::filesystem::path &path);
    void writeContent(const std::filesystem::path &path, const std::string &content);

public:
    ShaderResource(const std::filesystem::path &binPath, const std::filesystem::path &codePath,
                   RendererAllocator *rendererAllocator);
    ~ShaderResource() override = default;

    void load() override;
    void unload() override;

    void loadCode();
    void saveCode(const std::string &content);
    void unloadCode();

    [[nodiscard]] ResourceType type() const override { return SHADER_RESOURCE; }

    [[nodiscard]] ShaderObject *shader() const { return this->_shader; }
    [[nodiscard]] const std::vector<char> &shaderCode() const { return this->_shaderCode; }

    bool build();
};


#endif // RESOURCES_SHADERRESOURCE_HPP
