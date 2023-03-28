#include "SceneTreeWindow.hpp"

#include <imgui.h>

#include "src/Debug/DebugUIState.hpp"
#include "src/Debug/Strings.hpp"
#include "src/Scene/Scene.hpp"
#include "src/Scene/SceneManager.hpp"
#include "src/Scene/SceneNode.hpp"

std::shared_ptr<SceneNode> SceneTreeWindow::drawItem(const std::shared_ptr<SceneNode> &node) {
    const bool isSelected = !this->_selectedNode.expired() && this->_selectedNode.lock() == node;

    ImGuiTreeNodeFlags flags = 0;

    if (isSelected) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    if (node->descendants().empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    } else {
        flags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    }

    std::shared_ptr<SceneNode> selected = nullptr;

    if (ImGui::TreeNodeEx(node->displayName().c_str(), flags)) {
        for (const auto &child: node->descendants()) {
            std::shared_ptr<SceneNode> nodeSelection = drawItem(child);

            if (nodeSelection != nullptr) {
                selected = nodeSelection;
            }
        }

        ImGui::TreePop();
    }

    if (selected == nullptr && ImGui::IsItemClicked()) {
        return node;
    }

    return selected;
}

SceneTreeWindow::SceneTreeWindow(const std::shared_ptr<DebugUIState> &debugUIState,
                                 const std::shared_ptr<SceneManager> &sceneManager)
        : _debugUIState(debugUIState),
          _sceneManager(sceneManager) {
    //
}

void SceneTreeWindow::draw(bool *visible) {
    if (!ImGui::Begin(SCENE_TREE_WINDOW_TITLE, visible)) {
        ImGui::End();

        return;
    }

    if (this->_sceneManager->currentScene() == nullptr) {
        ImVec2 labelSize = ImGui::CalcTextSize(SCENE_TREE_SCENE_NOT_AVAILABLE);
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImGui::SetCursorPos(ImVec2(0.5f * (windowSize.x - labelSize.x),
                                   0.5f * (windowSize.y - labelSize.y)));

        ImGui::Text(SCENE_TREE_SCENE_NOT_AVAILABLE);

        ImGui::End();

        return;
    }

    std::shared_ptr<SceneNode> selected;

    if (ImGui::BeginListBox("##scene_tree", ImVec2(-1, -1))) {
        selected = this->drawItem(this->_sceneManager->currentScene()->root());

        ImGui::EndListBox();
    }

    if (selected != nullptr) {
        this->_selectedNode = selected;
        this->_debugUIState->object = selected->object();
        this->_debugUIState->objectEditorWindowVisible = true;
    }

    ImGui::End();
}
