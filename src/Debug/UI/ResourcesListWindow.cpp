#include "ResourcesListWindow.hpp"

#include <imgui.h>

#include "src/Debug/Strings.hpp"
#include "src/Resources/Resource.hpp"
#include "src/Resources/ResourceManager.hpp"

ResourcesListWindow::ResourcesListWindow(const std::shared_ptr<ResourceManager> &resourceManager)
        : _resourceManager(resourceManager) {
    //
}

void ResourcesListWindow::draw(bool *visible) {
    if (!ImGui::Begin(RESOURCES_LIST_WINDOW_TITLE, visible)) {
        ImGui::End();

        return;
    }

    if (ImGui::BeginTable("##resources", 4, ImGuiTableFlags_RowBg, ImVec2(-1, -1))) {
        ImGui::TableSetupColumn(RESOURCES_LIST_ID, ImGuiTableColumnFlags_WidthFixed, 300.0f);
        ImGui::TableSetupColumn(RESOURCES_LIST_TYPE, ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn(RESOURCES_LIST_LOADED, ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn(RESOURCES_LIST_PATHS, ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();

        for (const auto &[id, resource]: this->_resourceManager->resources()) {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text("%s", id.c_str());

            ImGui::TableNextColumn();
            std::string type = toString(resource->type());
            ImGui::Text("%s", type.c_str());

            ImGui::TableNextColumn();
            ImGui::Text("%s", resource->isLoaded() ? "yes" : "no");

            ImGui::TableNextColumn();
            for (uint32_t idx = 0; idx < resource->paths().size(); idx++) {
                ImGui::Text("%d: %s", idx, resource->paths()[idx].c_str());
            }
        }

        ImGui::EndTable();
    }

    ImGui::End();
}
