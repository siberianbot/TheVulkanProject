#include "DebugUI.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "src/Engine.hpp"
#include "src/Events/EventQueue.hpp"
#include "src/Scene/Light.hpp"
#include "src/Scene/Object.hpp"
#include "src/Scene/Scene.hpp"

void DebugUI::renderMainMenu() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Engine")) {
            if (ImGui::MenuItem("FPS", NULL, this->_engineFpsWindowVisible)) {
                this->_engineFpsWindowVisible = !this->_engineFpsWindowVisible;
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Exit")) {
                this->_engine->eventQueue()->pushEvent(Event{.type = CLOSE_REQUESTED_EVENT});
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Renderer")) {
            if (ImGui::MenuItem("Reload renderpasses")) {
                this->_engine->eventQueue()->pushEvent(Event{.type = RENDERER_RELOADING_REQUESTED_EVENT});
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Scene")) {
            if (ImGui::MenuItem("Objects", NULL, this->_sceneObjectsWindowVisible)) {
                this->_sceneObjectsWindowVisible = !this->_sceneObjectsWindowVisible;
            }

            if (ImGui::MenuItem("Lights", NULL, this->_sceneLightsWindowVisible)) {
                this->_sceneLightsWindowVisible = !this->_sceneLightsWindowVisible;
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void DebugUI::renderEngineFpsWindow() {
    if (!this->_engineFpsWindowVisible) {
        return;
    }

    int offset = 10;
    ImVec2 size = ImVec2(200, 4 * ImGui::GetTextLineHeightWithSpacing());
    ImVec2 pos = ImVec2(this->_engine->windowWidth() - size.x - offset,
                        this->_engine->windowHeight() - size.y - offset);

    ImGui::SetNextWindowSize(size);
    ImGui::SetNextWindowPos(pos);

    if (ImGui::Begin("FPS", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                     ImGuiWindowFlags_NoCollapse)) {
        float delta = this->_engine->delta();

        ImGui::Text("Frame time: %.3f ms", delta * 1000);
        ImGui::Text("Frames per second: %.0f", 1.0f / delta);
        ImGui::End();
    }
}

void DebugUI::renderSceneObjectsWindow() {
    if (!this->_sceneObjectsWindowVisible) {
        return;
    }

    if (ImGui::Begin("Objects")) {
        if (ImGui::Button("Add object")) {
            Object *object = new Object(glm::vec3(0), glm::vec3(0), glm::vec3(1),
                                        this->_engine->cubeMeshResource(),
                                        this->_engine->cubeTextureResource());

            this->_engine->scene()->addObject(object);
            this->_selectedObject = std::nullopt;
        }

        ImGui::SameLine();

        if (ImGui::Button("Remove all objects")) {
            this->_engine->scene()->clearObjects();
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

            ImGui::InputScalarN("Position", ImGuiDataType_Float, reinterpret_cast<float *>(&object->position()),
                                3, &this->_floatStep, &this->_floatFastStep, "%.3f");
            ImGui::InputScalarN("Rotation", ImGuiDataType_Float, reinterpret_cast<float *>(&object->rotation()),
                                3, &this->_floatStep, &this->_floatFastStep, "%.3f");
            ImGui::InputScalarN("Scale", ImGuiDataType_Float, reinterpret_cast<float *>(&object->scale()),
                                3, &this->_floatStep, &this->_floatFastStep, "%.3f");

            if (ImGui::Button("Delete object")) {
                this->_engine->scene()->removeObject(object);
                this->_selectedObject = std::nullopt;
            }
        }

        ImGui::End();
    }
}

void DebugUI::renderSceneLightsWindow() {
    if (!this->_sceneLightsWindowVisible) {
        return;
    }

    if (ImGui::Begin("Lights")) {
        if (ImGui::Button("Add light")) {
            Light *light = new Light();

            this->_engine->scene()->addLight(light);
            this->_selectedLight = std::nullopt;
        }

        ImGui::SameLine();

        if (ImGui::Button("Remove all lights")) {
            this->_engine->scene()->clearLights();
            this->_selectedLight = std::nullopt;
        }

        if (ImGui::BeginListBox("#lights", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing()))) {
            for (auto current = this->_engine->scene()->lights().begin();
                 current != this->_engine->scene()->lights().end(); ++current) {
                const bool isSelected = (this->_selectedLight == current);

                std::string name = std::string("light ") + std::to_string(reinterpret_cast<long>(*current));

                if (ImGui::Selectable(name.c_str(), isSelected)) {
                    this->_selectedLight = current;
                }

                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndListBox();
        }

        if (this->_selectedLight.has_value()) {
            Light *light = *this->_selectedLight.value();

            ImGui::InputScalarN("Position", ImGuiDataType_Float, reinterpret_cast<float *>(&light->position()),
                                3, &this->_floatStep, &this->_floatFastStep, "%.3f");
            ImGui::InputScalarN("Rotation", ImGuiDataType_Float, reinterpret_cast<float *>(&light->rotation()),
                                2, &this->_floatStep, &this->_floatFastStep, "%.3f");
            ImGui::ColorPicker3("Color", reinterpret_cast<float *>(&light->color()));
            ImGui::InputFloat("Radius", &light->radius(), this->_floatStep, this->_floatFastStep);

            if (ImGui::Button("Delete light")) {
                this->_engine->scene()->removeLight(light);
                this->_selectedLight = std::nullopt;
            }
        }

        ImGui::End();
    }
}

DebugUI::DebugUI(Engine *engine)
        : _engine(engine) {
    //
}

void DebugUI::render() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    renderMainMenu();
    renderEngineFpsWindow();
    renderSceneObjectsWindow();
    renderSceneLightsWindow();
}
