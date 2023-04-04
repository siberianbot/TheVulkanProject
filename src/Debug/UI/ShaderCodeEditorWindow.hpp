#ifndef DEBUG_UI_SHADERCODEEDITORWINDOW_HPP
#define DEBUG_UI_SHADERCODEEDITORWINDOW_HPP

#include <memory>
#include <string>

#include "src/Debug/UI/WindowBase.hpp"
#include "src/Resources/ResourceId.hpp"

class Resource;
class ResourceDatabase;

class ShaderCodeEditorWindow : public WindowBase {
private:
    std::shared_ptr<ResourceDatabase> _resourceDatabase;

    std::string _code;
    ResourceId _selectedId;
    std::weak_ptr<Resource> _selectedResource;

    void selectShader(const std::shared_ptr<Resource> &resource);

public:
    ShaderCodeEditorWindow(const std::shared_ptr<ResourceDatabase> &resourceDatabase);
    ~ShaderCodeEditorWindow() override = default;

    void draw(bool *visible) override;
};


#endif // DEBUG_UI_SHADERCODEEDITORWINDOW_HPP
