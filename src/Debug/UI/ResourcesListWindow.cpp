#include "ResourcesListWindow.hpp"

#include <imgui.h>

#include "src/Debug/Strings.hpp"
#include "src/Resources/Resource.hpp"
#include "src/Resources/ResourceDatabase.hpp"

ResourcesListWindow::ResourcesListWindow(const std::shared_ptr<ResourceDatabase> &resourceDatabase)
        : _resourceDatabase(resourceDatabase) {
    //
}

void ResourcesListWindow::draw(bool *visible) {
    if (!ImGui::Begin(RESOURCES_LIST_WINDOW_TITLE, visible)) {
        ImGui::End();

        return;
    }

    if (ImGui::BeginTable("##resources", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable, ImVec2(-1, -1))) {
        ImGui::TableSetupColumn(RESOURCES_LIST_ID, ImGuiTableColumnFlags_WidthFixed, 300.0f);
        ImGui::TableSetupColumn(RESOURCES_LIST_TYPE, ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn(RESOURCES_LIST_PATHS, ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        for (const auto &[id, resource]: this->_resourceDatabase->resources()) {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::TextWrapped("%s", id.c_str());

            ImGui::TableNextColumn();
            ImGui::Text("%s", toString(resource->type()).c_str());

            ImGui::TableNextColumn();
            for (uint32_t idx = 0; idx < resource->paths().size(); idx++) {
                ImGui::TextWrapped("%d: %s", idx, resource->paths()[idx].c_str());
            }
        }

        ImGui::EndTable();
    }

    ImGui::End();
}
