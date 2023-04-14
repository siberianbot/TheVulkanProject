#include "ResourcesListWindow.hpp"

#include <imgui.h>

#include "src/Debug/Strings.hpp"
#include "src/Resources/Resource.hpp"
#include "src/Resources/ResourceDatabase.hpp"
#include "src/Resources/ResourceLoader.hpp"

ResourcesListWindow::ResourcesListWindow(const std::shared_ptr<ResourceDatabase> &resourceDatabase,
                                         const std::shared_ptr<ResourceLoader> &resourceLoader)
        : _resourceDatabase(resourceDatabase),
          _resourceLoader(resourceLoader) {
    //
}

void ResourcesListWindow::draw(bool *visible) {
    if (!ImGui::Begin(RESOURCES_LIST_WINDOW_TITLE, visible)) {
        ImGui::End();

        return;
    }

    if (ImGui::BeginTable("##resources", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable, ImVec2(-1, -1))) {
        ImGui::TableSetupColumn(RESOURCES_LIST_ID, ImGuiTableColumnFlags_WidthFixed, 300.0f);
        ImGui::TableSetupColumn(RESOURCES_LIST_TYPE, ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn(RESOURCES_LIST_LOADED, ImGuiTableColumnFlags_WidthFixed, 50.0f);
        ImGui::TableSetupColumn(RESOURCES_LIST_PATHS, ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        for (const auto &[id, resource]: this->_resourceDatabase->resources()) {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::TextWrapped("%s", id.c_str());

            ImGui::TableNextColumn();
            ImGui::Text("%s", toString(resource->type()).data());

            ImGui::TableNextColumn();
            ImGui::Text("%s", this->_resourceLoader->isLoaded(id) ? YES : NO);

            ImGui::TableNextColumn();
            ImGui::TextWrapped("%s", resource->path().c_str());
        }

        ImGui::EndTable();
    }

    ImGui::End();
}
