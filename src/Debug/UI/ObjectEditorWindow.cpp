#include "ObjectEditorWindow.hpp"

#include <imgui.h>

#include "src/Debug/DebugUIState.hpp"
#include "src/Debug/Strings.hpp"
#include "src/Debug/UI/ObjectEditVisitor.hpp"
#include "src/Objects/Object.hpp"
#include "src/Objects/Components/Component.hpp"

ObjectEditorWindow::ObjectEditorWindow(const std::shared_ptr<DebugUIState> &debugUIState,
                                       const std::shared_ptr<ResourceManager> &resourceManager)
        : _debugUIState(debugUIState),
          _objectEditVisitor(std::make_shared<ObjectEditVisitor>(resourceManager)) {
    //
}

void ObjectEditorWindow::draw(bool *visible) {
    if (!ImGui::Begin("Object Editor", visible)) {
        ImGui::End();

        return;
    }

    if (this->_debugUIState->object.expired()) {
        ImVec2 labelSize = ImGui::CalcTextSize("Object not selected");
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImGui::SetCursorPos(ImVec2(0.5f * (windowSize.x - labelSize.x),
                                   0.5f * (windowSize.y - labelSize.y)));

        ImGui::Text("Object not selected");

        ImGui::End();

        return;
    }

    std::shared_ptr<Object> object = this->_debugUIState->object.lock();
    ImGui::Text("Object %s", object->displayName().c_str());

    ImGui::Separator();

    for (std::shared_ptr<Component> component: object->components()) {
        component->acceptEdit(this->_objectEditVisitor);

        ImGui::Separator();
    }

    object->acceptEdit(this->_objectEditVisitor);

    ImGui::End();
}
