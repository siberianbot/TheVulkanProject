#ifndef DEBUG_UI_SHADERCODEEDITORWINDOW_HPP
#define DEBUG_UI_SHADERCODEEDITORWINDOW_HPP

#include <memory>
#include <string>

#include "src/Debug/UI/WindowBase.hpp"

class ResourceManager;
class ShaderResource;

class ShaderCodeEditorWindow : public WindowBase {
private:
    std::shared_ptr<ResourceManager> _resourceManager;

    std::string _selectedShaderId;
    std::string _selectedShaderCode;
    std::weak_ptr<ShaderResource> _selectedShaderResource;

    void selectShader(const std::shared_ptr<ShaderResource> &resource);

public:
    ShaderCodeEditorWindow(const std::shared_ptr<ResourceManager> &resourceManager);
    ~ShaderCodeEditorWindow() override = default;

    void draw(bool *visible) override;
};


#endif // DEBUG_UI_SHADERCODEEDITORWINDOW_HPP
