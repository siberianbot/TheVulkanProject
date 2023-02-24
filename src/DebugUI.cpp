#include "DebugUI.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "src/Engine.hpp"
#include "src/Scene/Object.hpp"
#include "src/Scene/Scene.hpp"

DebugUI::DebugUI(Engine *engine)
        : _engine(engine) {
    //
}

void DebugUI::update() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(this->_engine->windowWidth() - 160, this->_engine->windowHeight() - 80));
    ImGui::SetNextWindowSize(ImVec2(150, 70));
    ImGui::Begin("FPS", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    ImGui::Text("delta: %f", this->_engine->delta());
    ImGui::Text("fps: %f", 1.0f / this->_engine->delta());
    ImGui::End();

    if (ImGui::Begin("Objects")) {
        if (ImGui::Button("add")) {
            Object *object = new Object(glm::vec3(0), glm::vec3(0), glm::vec3(1),
                                        this->_engine->meshResource(), this->_engine->textureResource());

            this->_engine->scene()->addObject(object);
            this->_selectedObject = std::nullopt;
        }

        if (ImGui::Button("clear")) {
            this->_engine->scene()->clear();
            this->_selectedObject = std::nullopt;
        }

        if (ImGui::BeginListBox("#objects", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing()))) {
            for (auto current = this->_engine->scene()->objects().begin();
                 current != this->_engine->scene()->objects().end(); ++current) {
                const bool isSelected = (this->_selectedObject == current);

                std::string name = std::string("object ") + std::to_string(reinterpret_cast<long>(*current));

                if (ImGui::Selectable(name.c_str(), isSelected)) {
                    this->_selectedObject = current;
                }

                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndListBox();
        }

        if (this->_selectedObject.has_value()) {
            Object *object = *this->_selectedObject.value();

            // TODO: magic
            ImGui::InputFloat3("position", reinterpret_cast<float *>(&object->position()));
            ImGui::InputFloat3("rotation", reinterpret_cast<float *>(&object->rotation()));
            ImGui::InputFloat3("scale", reinterpret_cast<float *>(&object->scale()));

            if (ImGui::Button("delete")) {
                this->_engine->scene()->removeObject(object);
                this->_selectedObject = std::nullopt;
            }
        }

        ImGui::End();
    }
}
