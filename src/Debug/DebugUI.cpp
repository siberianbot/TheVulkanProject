#include "DebugUI.hpp"

#include <fstream>
#include <sstream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "src/Engine/Engine.hpp"
#include "src/Engine/EngineVars.hpp"
#include "src/Events/EventQueue.hpp"
#include "src/Resources/Resource.hpp"
#include "src/Resources/ResourceManager.hpp"
#include "src/Resources/MeshResource.hpp"
#include "src/Resources/ImageResource.hpp"
#include "src/Resources/ShaderResource.hpp"
#include "src/Objects/Light.hpp"
#include "src/Objects/Object.hpp"
#include "src/Scene/Scene.hpp"
#include "src/Scene/SceneManager.hpp"
#include "src/System/Window.hpp"

static constexpr const uint32_t TEXT_BUFFER_SIZE = 64 * 1024;

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
    ImVec2 pos = ImVec2(this->_engine->window()->width() - size.x - offset,
                        this->_engine->window()->height() - size.y - offset);

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
        if (ImGui::BeginCombo("Current shader", this->_selectedShaderName.c_str())) {
            // None
            {
                const bool selected = this->_selectedShaderResource == nullptr;

                if (ImGui::Selectable(NONE_ITEM, selected)) {
                    if (this->_selectedShaderResource != nullptr) {
                        this->_selectedShaderResource->unloadCode();
                    }

                    this->_selectedShaderResource = nullptr;
                    this->_selectedShaderName = NONE_ITEM;
                    this->_selectedShaderCode.clear();
                    this->_selectedShaderCode.resize(0);
                    this->_selectedShaderStatus.clear();
                }

                if (selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }

            for (const auto &[id, resource]: this->_engine->resourceManager()->resources()) {
                if (resource->type() != SHADER_RESOURCE) {
                    continue;
                }

                const bool selected = this->_selectedShaderResource == resource;

                if (ImGui::Selectable(id.c_str(), selected)) {
                    if (this->_selectedShaderResource != nullptr) {
                        this->_selectedShaderResource->unloadCode();
                    }

                    this->_selectedShaderResource = std::dynamic_pointer_cast<ShaderResource>(resource);

                    try {
                        this->_selectedShaderResource->loadCode();
                    } catch (std::runtime_error &error) {
                        this->_selectedShaderStatus = "Failed to load shader code";
                        this->_selectedShaderResource = nullptr;
                    }

                    if (this->_selectedShaderResource != nullptr) {
                        this->_selectedShaderName = id;

                        uint32_t size = TEXT_BUFFER_SIZE + this->_selectedShaderResource->shaderCode().size();
                        this->_selectedShaderCode.clear();
                        this->_selectedShaderCode.resize(size);

                        std::copy(this->_selectedShaderResource->shaderCode().begin(),
                                  this->_selectedShaderResource->shaderCode().end(),
                                  this->_selectedShaderCode.begin());

                        this->_selectedShaderStatus.clear();
                    }
                }

                if (selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }

        ImGui::InputTextMultiline("Shader Code", this->_selectedShaderCode.data(), this->_selectedShaderCode.size(),
                                  ImVec2(-1, -1));

        ImGui::Text("%s", this->_selectedShaderStatus.c_str());

        if (this->_selectedShaderResource != nullptr) {
            if (ImGui::Button("Save")) {
                try {
                    this->_selectedShaderResource->saveCode(this->_selectedShaderCode);
                } catch (std::runtime_error &error) {
                    this->_selectedShaderStatus = "Failed to save shader code";
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Build")) {
                this->_selectedShaderStatus = this->_selectedShaderResource->build()
                                              ? "Successfully built"
                                              : "Failed to build shader code";
            }
        }
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
            std::shared_ptr<MeshResource> cubeMesh = this->_engine->resourceManager()->loadMesh("cube");
            std::shared_ptr<ImageResource> defaultImage = this->_engine->resourceManager()->loadDefaultImage();

            Object *object = new Object();
            object->mesh() = cubeMesh;
            object->albedoTexture() = defaultImage;

            this->_selectedObjectMeshName = this->_selectedObject->mesh()->id();
            this->_selectedObjectAlbedoTextureName = this->_selectedObject->albedoTexture()->id();
            this->_selectedObjectSpecularTextureName = this->_selectedObject->specTexture()->id();

            currentScene->addObject(object);

            this->_selectedObject = object;
        }

        ImGui::SameLine();

        if (ImGui::Button("Remove all objects")) {
            currentScene->clearObjects();
            this->_selectedObject = nullptr;
        }

        if (ImGui::BeginListBox("#objects", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing()))) {
            for (Object *object: currentScene->objects()) {
                const bool isSelected = (this->_selectedObject == object);

                std::string name = std::string("object ") + std::to_string(reinterpret_cast<long>(object));
                if (ImGui::Selectable(name.c_str(), isSelected)) {
                    this->_selectedObject = object;

                    this->_selectedObjectMeshName = this->_selectedObject->mesh()->id();
                    this->_selectedObjectAlbedoTextureName = this->_selectedObject->albedoTexture()->id();
                    this->_selectedObjectSpecularTextureName = this->_selectedObject->specTexture()->id();
                }

                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndListBox();
        }

        if (this->_selectedObject != nullptr) {
            ImGui::InputScalarN("Position", ImGuiDataType_Float, reinterpret_cast<float *>(
                    &this->_selectedObject->position()), 3, &this->_floatStep, &this->_floatFastStep, "%.3f");

            ImGui::SliderAngle("Pitch", &this->_selectedObject->rotation().x, 0, 360.0f);
            ImGui::SliderAngle("Yaw", &this->_selectedObject->rotation().y, 0, 360.0f);
            ImGui::SliderAngle("Roll", &this->_selectedObject->rotation().z, 0, 360.0f);

            if (ImGui::BeginCombo("Mesh", this->_selectedObjectMeshName.c_str())) {
                // None
                {
                    const bool selected = this->_selectedObject->mesh() == nullptr;

                    if (ImGui::Selectable(NONE_ITEM, selected)) {
                        this->_selectedObject->mesh() = nullptr;
                        this->_selectedObjectMeshName = NONE_ITEM;
                    }

                    if (selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }

                for (const auto &[id, resource]: this->_engine->resourceManager()->resources()) {
                    if (resource->type() != MESH_RESOURCE) {
                        continue;
                    }

                    const bool selected = this->_selectedObject->mesh() == resource;

                    if (ImGui::Selectable(id.c_str(), selected)) {
                        this->_selectedObject->mesh() = std::dynamic_pointer_cast<MeshResource>(resource);
                        this->_selectedObjectMeshName = id;
                    }

                    if (selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }

                ImGui::EndCombo();
            }

            if (ImGui::BeginCombo("Albedo texture", this->_selectedObjectAlbedoTextureName.c_str())) {
                // None
                {
                    const bool selected = this->_selectedObject->albedoTexture() == nullptr;

                    if (ImGui::Selectable(NONE_ITEM, selected)) {
                        this->_selectedObject->albedoTexture() = nullptr;
                        this->_selectedObjectAlbedoTextureName = NONE_ITEM;
                    }

                    if (selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }

                for (const auto &[id, resource]: this->_engine->resourceManager()->resources()) {
                    if (resource->type() != IMAGE_RESOURCE) {
                        continue;
                    }

                    const bool selected = this->_selectedObject->albedoTexture() == resource;

                    if (ImGui::Selectable(id.c_str(), selected)) {
                        this->_selectedObject->albedoTexture() = std::dynamic_pointer_cast<ImageResource>(resource);
                        this->_selectedObjectAlbedoTextureName = id;
                    }

                    if (selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }

                ImGui::EndCombo();
            }

            if (ImGui::BeginCombo("Specular texture", this->_selectedObjectSpecularTextureName.c_str())) {
                // None
                {
                    const bool selected = this->_selectedObject->specTexture() == nullptr;

                    if (ImGui::Selectable(NONE_ITEM, selected)) {
                        this->_selectedObject->specTexture() = nullptr;
                        this->_selectedObjectSpecularTextureName = NONE_ITEM;
                    }

                    if (selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }

                for (const auto &[id, resource]: this->_engine->resourceManager()->resources()) {
                    if (resource->type() != IMAGE_RESOURCE) {
                        continue;
                    }

                    const bool selected = this->_selectedObject->specTexture() == resource;

                    if (ImGui::Selectable(id.c_str(), selected)) {
                        this->_selectedObject->specTexture() = std::dynamic_pointer_cast<ImageResource>(resource);
                        this->_selectedObjectSpecularTextureName = id;
                    }

                    if (selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }

                ImGui::EndCombo();
            }

            if (ImGui::Button("Delete object")) {
                currentScene->removeObject(this->_selectedObject);
                this->_selectedObject = nullptr;
                this->_selectedObjectMeshName = NONE_ITEM;
                this->_selectedObjectAlbedoTextureName = NONE_ITEM;
                this->_selectedObjectSpecularTextureName = NONE_ITEM;
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
        if (ImGui::BeginTable("#resources", 4, 0, ImVec2(-1, -1))) {
            ImGui::TableSetupColumn("Id");
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("Loaded");
            ImGui::TableSetupColumn("Path");
            ImGui::TableHeadersRow();

            for (auto &[id, resource]: this->_engine->resourceManager()->resources()) {
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::Text("%s", id.c_str());

                ImGui::TableNextColumn();
                std::string type = toString(resource->type());
                ImGui::Text("%s", type.c_str());

                ImGui::TableNextColumn();
                ImGui::Text("%s", resource->isLoaded() ? "yes" : "no");

                ImGui::TableNextColumn();
                for (auto &path: resource->paths()) {
                    ImGui::Text("%s", path.c_str());
                }
            }

            ImGui::EndTable();
        }
    }

    ImGui::End();
}

DebugUI::DebugUI(Engine *engine)
        : _engine(engine) {
    //
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
