#include "LogWindow.hpp"

#include <imgui.h>

#include "src/Debug/Strings.hpp"
#include "src/Engine/Log.hpp"

LogWindow::LogWindow(const std::shared_ptr<Log> &log)
        : _log(log) {
    //
}

void LogWindow::draw(bool *visible) {
    if (!ImGui::Begin(LOG_WINDOW_TITLE, visible)) {
        ImGui::End();

        return;
    }

    if (ImGui::BeginChild("##scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar)) {
        for (const auto &entry: this->_log->buffer()) {
            ImGui::Text("%d %s %s", entry.category, entry.tag.c_str(), entry.msg.c_str());
        }
    }

    ImGui::EndChild();

    ImGui::End();
}
