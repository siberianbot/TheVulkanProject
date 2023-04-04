#include "DebugUIRoot.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "src/Debug/DebugUIState.hpp"
#include "src/Debug/UI/LogWindow.hpp"
#include "src/Debug/UI/MainMenuBar.hpp"
#include "src/Debug/UI/ObjectEditorWindow.hpp"
#include "src/Debug/UI/ResourcesListWindow.hpp"
#include "src/Debug/UI/SceneTreeWindow.hpp"
#include "src/Debug/UI/ShaderCodeEditorWindow.hpp"
#include "src/Debug/UI/VariablesWindow.hpp"

DebugUIRoot::DebugUIRoot(const std::shared_ptr<Log> &log,
                         const std::shared_ptr<EventQueue> &eventQueue,
                         const std::shared_ptr<VarCollection> &vars,
                         const std::shared_ptr<ResourceDatabase> &resourceDatabase,
                         const std::shared_ptr<SceneManager> &sceneManager)
        : _state(std::make_shared<DebugUIState>()),
          _logWindow(std::make_shared<LogWindow>(log)),
          _mainMenuBar(std::make_shared<MainMenuBar>(this->_state, eventQueue)),
          _objectEditorWindow(std::make_shared<ObjectEditorWindow>(this->_state, resourceDatabase)),
          _resourceListWindow(std::make_shared<ResourcesListWindow>(resourceDatabase)),
          _sceneTreeWindow(std::make_shared<SceneTreeWindow>(this->_state, sceneManager)),
          _shaderCodeEditorWindow(std::make_shared<ShaderCodeEditorWindow>(resourceDatabase)),
          _variablesWindow(std::make_shared<VariablesWindow>(vars)) {
    //
}

void DebugUIRoot::render() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    this->_mainMenuBar->draw();

    if (this->_state->logWindowVisible) {
        this->_logWindow->draw(&this->_state->logWindowVisible);
    }

    if (this->_state->objectEditorWindowVisible) {
        this->_objectEditorWindow->draw(&this->_state->objectEditorWindowVisible);
    }

    if (this->_state->resourceListWindowVisible) {
        this->_resourceListWindow->draw(&this->_state->resourceListWindowVisible);
    }

    if (this->_state->sceneTreeWindowVisible) {
        this->_sceneTreeWindow->draw(&this->_state->sceneTreeWindowVisible);
    }

    if (this->_state->shaderCodeEditorWindowVisible) {
        this->_shaderCodeEditorWindow->draw(&this->_state->shaderCodeEditorWindowVisible);
    }

    if (this->_state->variablesWindowVisible) {
        this->_variablesWindow->draw(&this->_state->variablesWindowVisible);
    }
}
