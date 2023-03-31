#include "MainMenuBar.hpp"

#include <imgui.h>

#include "src/Debug/DebugUIState.hpp"
#include "src/Debug/Strings.hpp"
#include "src/Events/EventQueue.hpp"

MainMenuBar::MainMenuBar(const std::shared_ptr<DebugUIState> &debugUIState,
                         const std::shared_ptr<EventQueue> &eventQueue)
        : _debugUIState(debugUIState),
          _eventQueue(eventQueue) {
    //
}

void MainMenuBar::draw() {
    if (!ImGui::BeginMainMenuBar()) {
        ImGui::EndMainMenuBar();

        return;
    }

    if (ImGui::BeginMenu(MAIN_MENU_ENGINE)) {
        if (ImGui::MenuItem(LOG_WINDOW_TITLE, NULL, this->_debugUIState->logWindowVisible)) {
            this->_debugUIState->logWindowVisible = !this->_debugUIState->logWindowVisible;
        }

        if (ImGui::MenuItem(VARIABLES_WINDOW_TITLE, NULL, this->_debugUIState->variablesWindowVisible)) {
            this->_debugUIState->variablesWindowVisible = !this->_debugUIState->variablesWindowVisible;
        }

        ImGui::Separator();

        if (ImGui::MenuItem(MAIN_MENU_ENGINE_EXIT)) {
            this->_eventQueue->pushEvent(Event{.type = CLOSE_REQUESTED_EVENT});
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu(MAIN_MENU_RENDERING)) {
        if (ImGui::MenuItem(MAIN_MENU_RENDERING_RELOAD_RENDERER)) {
            this->_eventQueue->pushEvent(Event{.type = RENDERER_RELOADING_REQUESTED_EVENT});
        }

        ImGui::Separator();

        if (ImGui::MenuItem(SHADER_CODE_EDITOR_WINDOW_TITLE, NULL,
                            this->_debugUIState->shaderCodeEditorWindowVisible)) {
            this->_debugUIState->shaderCodeEditorWindowVisible = !this->_debugUIState->shaderCodeEditorWindowVisible;
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu(MAIN_MENU_SCENE)) {
        if (ImGui::MenuItem(SCENE_TREE_WINDOW_TITLE, NULL, this->_debugUIState->sceneTreeWindowVisible)) {
            this->_debugUIState->sceneTreeWindowVisible = !this->_debugUIState->sceneTreeWindowVisible;
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu(MAIN_MENU_RESOURCES)) {
        if (ImGui::MenuItem(RESOURCES_LIST_WINDOW_TITLE, NULL, this->_debugUIState->resourceListWindowVisible)) {
            this->_debugUIState->resourceListWindowVisible = !this->_debugUIState->resourceListWindowVisible;
        }

        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}
