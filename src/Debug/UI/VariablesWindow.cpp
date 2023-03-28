#include "VariablesWindow.hpp"

#include <imgui.h>

#include "src/Debug/Strings.hpp"
#include "src/Engine/VarCollection.hpp"

VariablesWindow::VariablesWindow(const std::shared_ptr<VarCollection> &vars)
        : _vars(vars) {
    //
}

void VariablesWindow::draw(bool *visible) {
    if (!ImGui::Begin(VARIABLES_WINDOW_TITLE, visible)) {
        ImGui::End();

        return;
    }

    if (ImGui::BeginTable("##vars", 2, ImGuiTableFlags_RowBg, ImVec2(-1, -1))) {
        ImGui::TableSetupColumn(VARIABLES_KEY);
        ImGui::TableSetupColumn(VARIABLES_VALUE);
        ImGui::TableHeadersRow();

        for (auto &[key, var]: this->_vars->vars()) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s", key.c_str());
            ImGui::TableNextColumn();

            std::string id = "##" + key;

            if (bool *b = std::get_if<bool>(&var)) {
                ImGui::Checkbox(id.c_str(), b);
            }

            if (int *i = std::get_if<int>(&var)) {
                ImGui::InputInt(id.c_str(), i);
            }

            if (float *f = std::get_if<float>(&var)) {
                ImGui::InputFloat(id.c_str(), f);
            }

            if (std::string *str = std::get_if<std::string>(&var)) {
                ImGui::Text("%s", str->c_str());
            }
        }

        ImGui::EndTable();
    }

    ImGui::End();
}
