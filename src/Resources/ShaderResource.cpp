#include "ShaderResource.hpp"

#include <fstream>
#include <sstream>

#include "src/Rendering/RendererAllocator.hpp"
#include "src/Rendering/Objects/ShaderObject.hpp"

std::vector<char> ShaderResource::readContent(const std::filesystem::path &path) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to read content");
    }

    size_t size = file.tellg();
    std::vector<char> buffer(size);

    file.seekg(0);
    file.read(buffer.data(), size);
    file.close();

    return buffer;
}

void ShaderResource::writeContent(const std::filesystem::path &path, const std::string &content) {
    std::ofstream file(path, std::ios::trunc);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to write content");
    }

    size_t size = strnlen(content.data(), content.size());

    file.write(content.data(), size);
    file.close();
}

ShaderResource::ShaderResource(const std::string &id,
                               const std::filesystem::path &binPath, const std::filesystem::path &codePath,
                               const std::shared_ptr<RendererAllocator> &rendererAllocator)
        : Resource(id, {binPath, codePath}),
          _rendererAllocator(rendererAllocator) {
    //
}

void ShaderResource::load() {
    if (this->_isLoaded) {
        return;
    }

    this->_shader = this->_rendererAllocator->uploadShaderBinary(this->readContent(this->_paths[0]));

    Resource::load();
}

void ShaderResource::unload() {
    if (!this->_isLoaded) {
        return;
    }

    if (this->_shader != nullptr) {
        this->_shader->destroy();
        this->_shader = nullptr;
    }

    this->unloadCode();

    Resource::unload();
}

void ShaderResource::loadCode() {
    this->_shaderCode = this->readContent(this->_paths[1]);
}

void ShaderResource::saveCode(const std::string &content) {
    this->writeContent(this->_paths[1], content);
}

void ShaderResource::unloadCode() {
    this->_shaderCode.clear();
}

bool ShaderResource::build() {
    std::stringstream cmdBuilder;
    cmdBuilder << "glslangValidator -gVS -V " << this->_paths[1] << " -o " << this->_paths[0];

    std::string cmd = cmdBuilder.str();

    return system(cmd.c_str()) == 0;
}
