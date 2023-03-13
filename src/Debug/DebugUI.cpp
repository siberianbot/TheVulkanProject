#include "DebugUI.hpp"

#include <fstream>
#include <sstream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "src/Engine.hpp"
#include "src/EngineVars.hpp"
#include "src/Events/EventQueue.hpp"
#include "src/Resources/ResourceManager.hpp"
#include "src/Objects/Light.hpp"
#include "src/Objects/Object.hpp"
#include "src/Scene/Scene.hpp"
#include "src/Scene/SceneManager.hpp"

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

            if (ImGui::MenuItem("Variables", NULL, this->_engineVarsWindowVisible & 1)) {
                this->_engineVarsWindowVisible++;
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

        if (ImGui::BeginMenu("Resources")) {
            if (ImGui::MenuItem("Resources list", NULL, this->_resourcesListWindowVisible & 1)) {
                this->_resourcesListWindowVisible++;
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

void DebugUI::drawEngineVarsWindow() {
    if ((this->_engineVarsWindowVisible & 1) == 0) {
        return;
    }

    if (ImGui::Begin("Variables")) {
        if (ImGui::BeginTable("#vars", 2, 0, ImVec2(-1, -1))) {
            ImGui::TableSetupColumn("Key");
            ImGui::TableSetupColumn("Value");
            ImGui::TableHeadersRow();

            for (auto &[key, var]: this->_engine->engineVars()->vars()) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", key.c_str());
                ImGui::TableNextColumn();

                switch (var->type) {
                    case BOOLEAN_VAR:
                        ImGui::Checkbox("value", &var->boolValue);
                        break;

                    case INTEGER_VAR:
                        ImGui::InputInt("value", &var->intValue);
                        break;

                    case FLOAT_VAR:
                        ImGui::InputFloat("value", &var->floatValue);
                        break;

                    case STRING_VAR:
                        // TODO: ImGui::InputText("value", &var->stringValue, );
                        ImGui::Text("%s", var->stringValue);
                        break;
                }
            }

            ImGui::EndTable();
        }
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
        Scene *currentScene = this->_engine->sceneManager()->currentScene();

        if (currentScene == nullptr) {
            ImGui::Text("Scene not available");
            ImGui::End();
            return;
        }

        if (ImGui::Button("Add object")) {
            Object *object = new Object();
            object->mesh() = &this->_engine->meshes()[0];
            object->albedoTexture() = &this->_engine->textures()[0];

            currentScene->addObject(object);
            this->_selectedObject = std::nullopt;
        }

        ImGui::SameLine();

        if (ImGui::Button("Remove all objects")) {
            currentScene->clearObjects();
            this->_selectedObject = std::nullopt;
        }

        if (ImGui::BeginListBox("#objects", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing()))) {
            for (auto current = currentScene->objects().begin();
                 current != currentScene->objects().end(); ++current) {
                const bool isSelected = (this->_selectedObject == current);

                std::string name = std::string("object ") + std::to_string(reinterpret_cast<long>(*current));

                if (ImGui::Selectable(name.c_str(), isSelected)) {
                    this->_selectedObject = current;

                    this->_selectedObjectMeshIdx = -1;
                    for (uint32_t idx = 0; idx < this->_engine->meshes().size(); idx++) {
                        if ((*current)->mesh() == &this->_engine->meshes()[idx]) {
                            this->_selectedObjectMeshIdx = idx;
                            break;
                        }
                    }

                    this->_selectedObjectTextureIdx = -1;
                    for (uint32_t idx = 0; idx < this->_engine->textures().size(); idx++) {
                        if ((*current)->albedoTexture() == &this->_engine->textures()[idx]) {
                            this->_selectedObjectTextureIdx = idx + 1;
                            break;
                        } else {
                            this->_selectedObjectTextureIdx = 0;
                        }
                    }

                    this->_selectedObjectSpecTextureIdx = -1;
                    for (uint32_t idx = 0; idx < this->_engine->textures().size(); idx++) {
                        if ((*current)->specTexture() == &this->_engine->textures()[idx]) {
                            this->_selectedObjectSpecTextureIdx = idx + 1;
                            break;
                        } else {
                            this->_selectedObjectSpecTextureIdx = 0;
                        }
                    }
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
            ImGui::SliderAngle("Pitch", &object->rotation().x, 0, 360.0f);
            ImGui::SliderAngle("Yaw", &object->rotation().y, 0, 360.0f);
            ImGui::SliderAngle("Roll", &object->rotation().z, 0, 360.0f);
            ImGui::InputScalarN("Scale", ImGuiDataType_Float, reinterpret_cast<float *>(&object->scale()),
                                3, &this->_floatStep, &this->_floatFastStep, "%.3f");

            if (ImGui::Combo("Mesh", &this->_selectedObjectMeshIdx, this->_meshes.data(), this->_meshes.size())) {
                if (this->_selectedObjectMeshIdx != -1) {
                    object->mesh() = &this->_engine->meshes()[this->_selectedObjectMeshIdx];
                }
            }

            if (ImGui::Combo("Texture", &this->_selectedObjectTextureIdx, this->_textures.data(),
                             this->_textures.size())) {
                if (this->_selectedObjectTextureIdx - 1 != -1) {
                    object->albedoTexture() = &this->_engine->textures()[this->_selectedObjectTextureIdx - 1];
                } else {
                    object->albedoTexture() = nullptr;
                }
            }

            if (ImGui::Combo("Specular", &this->_selectedObjectSpecTextureIdx, this->_textures.data(),
                             this->_textures.size())) {
                if (this->_selectedObjectSpecTextureIdx - 1 != -1) {
                    object->specTexture() = &this->_engine->textures()[this->_selectedObjectSpecTextureIdx - 1];
                } else {
                    object->specTexture() = nullptr;
                }
            }

            if (ImGui::Button("Delete object")) {
                currentScene->removeObject(object);
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
        Scene *currentScene = this->_engine->sceneManager()->currentScene();

        if (currentScene == nullptr) {
            ImGui::Text("Scene not available");
            ImGui::End();
            return;
        }

        if (ImGui::Button("Add light")) {
            Light *light = new Light();

            currentScene->addLight(light);
            this->_selectedLight = std::nullopt;
        }

        ImGui::SameLine();

        if (ImGui::Button("Remove all lights")) {
            currentScene->clearLights();
            this->_selectedLight = std::nullopt;
        }

        if (ImGui::BeginListBox("#lights", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing()))) {
            for (auto current = currentScene->lights().begin();
                 current != currentScene->lights().end(); ++current) {
                const bool isSelected = (this->_selectedLight == current);

                std::string name = std::string("light ") + std::to_string(reinterpret_cast<long>(*current));

                if (ImGui::Selectable(name.c_str(), isSelected)) {
                    this->_selectedLight = current;
                    this->_selectedLightKindIdx = (int) (*current)->kind();
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
            if (ImGui::Combo("Kind", &this->_selectedLightKindIdx, this->_lightKinds, 3)) {
                if (this->_selectedLightKindIdx >= 0) {
                    light->kind() = (LightKind) this->_selectedLightKindIdx;
                }
            }
            ImGui::InputScalarN("Position", ImGuiDataType_Float, reinterpret_cast<float *>(&light->position()),
                                3, &this->_floatStep, &this->_floatFastStep, "%.3f");
            ImGui::SliderAngle("Pitch", &light->rotation().x, 0, 360.0f);
            ImGui::SliderAngle("Yaw", &light->rotation().y, 0, 360.0f);
            ImGui::ColorPicker3("Color", reinterpret_cast<float *>(&light->color()));
            ImGui::InputFloat("Power", &light->radius(), this->_floatStep, this->_floatFastStep);

            switch (light->kind()) {
                case SPOT_LIGHT:
                    ImGui::SliderFloat("Field of View", &light->fov(), 0, glm::radians(180.0f));
                    break;

                case RECT_LIGHT:
                    ImGui::InputScalarN("Rectangle", ImGuiDataType_Float, reinterpret_cast<float *>(&light->rect()),
                                        2, &this->_floatStep, &this->_floatFastStep, "%.3f");
                    break;

                default:
                    break;
            }

            ImGui::InputFloat("Near", &light->near(), this->_floatStep, this->_floatFastStep);
            ImGui::InputFloat("Far", &light->far(), this->_floatStep, this->_floatFastStep);

            if (ImGui::Button("Delete light")) {
                currentScene->removeLight(light);
                this->_selectedLight = std::nullopt;
            }
        }
    }

    ImGui::End();
}

void DebugUI::drawResourcesListWindow() {
    if ((this->_resourcesListWindowVisible & 1) == 0) {
        return;
    }


    if (ImGui::Begin("Resources list")) {
        if (ImGui::BeginTable("#resources", 3, 0, ImVec2(-1, -1))) {
            ImGui::TableSetupColumn("Id");
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("Path");
            ImGui::TableHeadersRow();

            for (auto &[id, resource]: this->_engine->resourceManager()->resources()) {
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::Text("%s", id.c_str());

                ImGui::TableNextColumn();
                std::string type;
                switch (resource.type) {
                    case MODEL_OBJ_RESOURCE:
                        type = "model-obj";
                        break;

                    case SHADER_CODE_RESOURCE:
                        type = "shader-code";
                        break;

                    case SHADER_SPV_RESOURCE:
                        type = "shader-spv";
                        break;

                    case IMAGE_RESOURCE:
                        type = "image";
                        break;
                }
                ImGui::Text("%s", type.c_str());

                ImGui::TableNextColumn();
                ImGui::Text("%s", resource.path.c_str());
            }

            ImGui::EndTable();
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

    for (uint32_t idx = 0; idx < this->_engine->meshes().size(); idx++) {
        std::string name = std::string("mesh ") + std::to_string(reinterpret_cast<long>(&this->_engine->meshes()[idx]));
        char *ptr = new char[name.size()];

        memcpy(ptr, name.data(), name.size());

        this->_meshes.push_back(ptr);
    }

    this->_textures.push_back("none");

    for (uint32_t idx = 0; idx < this->_engine->textures().size(); idx++) {
        std::string name = std::string("texture ") +
                           std::to_string(reinterpret_cast<long>(&this->_engine->textures()[idx]));
        char *ptr = new char[name.size()];

        memcpy(ptr, name.data(), name.size());

        this->_textures.push_back(ptr);
    }
}

void DebugUI::render() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    drawMainMenu();
    drawEngineFpsWindow();
    drawEngineVarsWindow();
    drawRendererShaderEditor();
    drawSceneObjectsWindow();
    drawSceneLightsWindow();
    drawResourcesListWindow();
}
