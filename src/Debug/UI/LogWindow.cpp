#include "LogWindow.hpp"

#include <imgui.h>

#include "src/Debug/Strings.hpp"
#include "src/Engine/Log.hpp"

static constexpr const ImColor WARNING_LOG_CATEGORY_BG0 = ImColor(0.75f, 0.75f, 0.0f, 1.0f);
static constexpr const ImColor WARNING_LOG_CATEGORY_BG1 = ImColor(0.50f, 0.50f, 0.0f, 1.0f);
static constexpr const ImColor ERROR_LOG_CATEGORY_BG0 = ImColor(0.75f, 0.0f, 0.0f, 1.0f);
static constexpr const ImColor ERROR_LOG_CATEGORY_BG1 = ImColor(0.50f, 0.0f, 0.0f, 1.0f);

LogWindow::LogWindow(const std::shared_ptr<Log> &log)
        : _log(log) {
    //
}

void LogWindow::draw(bool *visible) {
    if (!ImGui::Begin(LOG_WINDOW_TITLE, visible)) {
        ImGui::End();

        return;
    }

    if (ImGui::BeginTable("##log", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable, ImVec2(-1, -1))) {
        ImGui::TableSetupColumn(LOG_CATEGORY, ImGuiTableColumnFlags_WidthFixed, 75.0f);
        ImGui::TableSetupColumn(LOG_TAG, ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn(LOG_MESSAGE, ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        for (const auto &entry: this->_log->buffer()) {
            ImGui::TableNextRow();

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
            switch (entry.category) {
                case WARNING_LOG_CATEGORY:
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, WARNING_LOG_CATEGORY_BG0);
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, WARNING_LOG_CATEGORY_BG1);
                    break;

                case ERROR_LOG_CATEGORY:
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ERROR_LOG_CATEGORY_BG0);
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ERROR_LOG_CATEGORY_BG1);
                    break;
            }
#pragma clang diagnostic pop

            ImGui::TableNextColumn();
            ImGui::TextWrapped("%s", toString(entry.category).c_str());

            ImGui::TableNextColumn();
            ImGui::TextWrapped("%s", entry.tag.c_str());

            ImGui::TableNextColumn();
            ImGui::TextWrapped("%s", entry.msg.c_str());
        }

        ImGui::EndTable();
    }

    ImGui::End();
}
