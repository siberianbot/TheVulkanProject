#include "SceneTreeWindow.hpp"

#include <imgui.h>

#include "src/Debug/Strings.hpp"
#include "src/Scene/Scene.hpp"
#include "src/Scene/SceneManager.hpp"
#include "src/Scene/SceneNode.hpp"

void SceneTreeWindow::renderItem(const std::shared_ptr<SceneNode> &node) {
    const bool isSelected = !this->_selectedNode.expired() && this->_selectedNode.lock() == node;

    ImGuiTreeNodeFlags flags = 0;

    if (isSelected) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    if (node->descendants().empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    if (ImGui::TreeNodeEx(node->displayName().c_str(), flags)) {
        for (const auto &child: node->descendants()) {
            renderItem(child);
        }

        if (ImGui::IsItemClicked()) {
            // TODO: misclick
            this->_selectedNode = node;
        }

        ImGui::TreePop();
    }
}

SceneTreeWindow::SceneTreeWindow(const std::shared_ptr<SceneManager> &sceneManager)
        : _sceneManager(sceneManager) {
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

        ImGui::End();

        return;
    }

    if (ImGui::BeginListBox("##scene_tree", ImVec2(-1, -1))) {
        this->renderItem(this->_sceneManager->currentScene()->root());

        ImGui::EndListBox();
    }

    ImGui::End();
}
