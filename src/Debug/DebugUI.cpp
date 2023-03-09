#include "DebugUI.hpp"

#include <fstream>
#include <sstream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "src/Engine.hpp"
#include "src/Events/EventQueue.hpp"
#include "src/Scene/Light.hpp"
#include "src/Scene/Object.hpp"
#include "src/Scene/Scene.hpp"

void DebugUI::loadShader(const char *path) {
    std::ifstream shader(path);

    if (!shader.is_open()) {
        return;
    }

    shader.seekg(0, std::ios::end);
    size_t size = shader.tellg();

    this->_shaderCode.resize(64 * 1024 + size);

    shader.seekg(0, std::ios::beg);
    shader.read(this->_shaderCode.data(), size);

    shader.close();
}

void DebugUI::saveShader(const char *path) {
    std::ofstream shader(path, std::ios::trunc);

    if (!shader.is_open()) {
        return;
    }

    size_t size = strnlen(this->_shaderCode.data(), this->_shaderCode.size());

    shader.write(this->_shaderCode.data(), size);
    shader.close();
}

void DebugUI::buildShader(const char *path) {
    std::stringstream cmdBuilder;
    cmdBuilder << "glslangValidator -gVS -V " << path << " -o " << path << ".spv";

    std::string cmd = cmdBuilder.str();

    this->_shaderBuildResult = system(cmd.c_str());
}

void DebugUI::drawMainMenu() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Engine")) {
            if (ImGui::MenuItem("FPS", NULL, this->_engineFpsWindowVisible & 1)) {
                this->_engineFpsWindowVisible++;
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

            ImGui::Separator();

            if (ImGui::MenuItem("Shader editor", NULL, this->_rendererShaderEditorVisible & 1)) {
                this->_rendererShaderEditorVisible++;
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Scene")) {
            if (ImGui::MenuItem("Objects", NULL, this->_sceneObjectsWindowVisible & 1)) {
                this->_sceneObjectsWindowVisible++;
            }

            if (ImGui::MenuItem("Lights", NULL, this->_sceneLightsWindowVisible & 1)) {
                this->_sceneLightsWindowVisible++;
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void DebugUI::drawEngineFpsWindow() {
    if ((this->_engineFpsWindowVisible & 1) == 0) {
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
    }
    
    ImGui::End();
}

void DebugUI::drawRendererShaderEditor() {
    if ((this->_rendererShaderEditorVisible & 1) == 0) {
        return;
    }

    if (ImGui::Begin("Shader editor")) {
        if (ImGui::Combo("Current Shader", &this->_selectedShaderIdx, this->_shaders.data(), this->_shaders.size())) {
            if (this->_selectedShaderIdx >= 0) {
                this->loadShader(this->_shaders[this->_selectedShaderIdx]);
            }
        }

        if (ImGui::Button("Save")) {
            if (this->_selectedShaderIdx >= 0) {
                this->saveShader(this->_shaders[this->_selectedShaderIdx]);
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Build")) {
            if (this->_selectedShaderIdx >= 0) {
                this->buildShader(this->_shaders[this->_selectedShaderIdx]);
            }
        }

        if (this->_shaderBuildResult == 0) {
            ImGui::Text("Successfully built!");
        } else if (this->_shaderBuildResult != -1) {
            ImGui::Text("Failed to build!");
        }

        ImGui::InputTextMultiline("Shader Code", this->_shaderCode.data(), this->_shaderCode.size(), ImVec2(-1, -1));
    }

    ImGui::End();
}

void DebugUI::drawSceneObjectsWindow() {
    if ((this->_sceneObjectsWindowVisible & 1) == 0) {
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
    }

    ImGui::End();
}

void DebugUI::drawSceneLightsWindow() {
    if ((this->_sceneLightsWindowVisible & 1) == 0) {
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

            ImGui::Checkbox("Enabled", &light->enabled());
            ImGui::InputScalarN("Position", ImGuiDataType_Float, reinterpret_cast<float *>(&light->position()),
                                3, &this->_floatStep, &this->_floatFastStep, "%.3f");
            ImGui::InputScalarN("Rotation", ImGuiDataType_Float, reinterpret_cast<float *>(&light->rotation()),
                                2, &this->_floatStep, &this->_floatFastStep, "%.3f");
            ImGui::ColorPicker3("Color", reinterpret_cast<float *>(&light->color()));
            ImGui::InputFloat("Radius", &light->radius(), this->_floatStep, this->_floatFastStep);
            ImGui::InputFloat("Field of View", &light->fov(), this->_floatStep, this->_floatFastStep);
            ImGui::InputFloat("Near", &light->near(), this->_floatStep, this->_floatFastStep);
            ImGui::InputFloat("Far", &light->far(), this->_floatStep, this->_floatFastStep);

            if (ImGui::Button("Delete light")) {
                this->_engine->scene()->removeLight(light);
                this->_selectedLight = std::nullopt;
            }
        }
    }

    ImGui::End();
}

DebugUI::DebugUI(Engine *engine)
        : _engine(engine) {
    this->_shaders.push_back("data/shaders/composition.frag");
    this->_shaders.push_back("data/shaders/composition.vert");
    this->_shaders.push_back("data/shaders/default.frag");
    this->_shaders.push_back("data/shaders/default.vert");
    this->_shaders.push_back("data/shaders/scene_composition.frag");
    this->_shaders.push_back("data/shaders/shadow.frag");
    this->_shaders.push_back("data/shaders/shadow.vert");
    this->_shaders.push_back("data/shaders/skybox.frag");
}

void DebugUI::render() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    drawMainMenu();
    drawEngineFpsWindow();
    drawRendererShaderEditor();
    drawSceneObjectsWindow();
    drawSceneLightsWindow();
}
