#include "DebugUI.hpp"

#include <fstream>
#include <functional>
#include <sstream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "src/Engine/EngineVars.hpp"
#include "src/Events/EventQueue.hpp"
#include "src/Resources/Resource.hpp"
#include "src/Resources/ResourceManager.hpp"
#include "src/Resources/MeshResource.hpp"
#include "src/Resources/ImageResource.hpp"
#include "src/Resources/ShaderResource.hpp"
#include "src/Objects/Object.hpp"
#include "src/Scene/Scene.hpp"
#include "src/Scene/SceneNode.hpp"
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
                this->_eventQueue->pushEvent(Event{.type = CLOSE_REQUESTED_EVENT});
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Renderer")) {
            if (ImGui::MenuItem("Reload renderpasses")) {
                this->_eventQueue->pushEvent(Event{.type = RENDERER_RELOADING_REQUESTED_EVENT});
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Shader editor", NULL, this->_rendererShaderEditorVisible & 1)) {
                this->_rendererShaderEditorVisible++;
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Scene")) {
            if (ImGui::MenuItem("Scene tree", NULL, this->_sceneObjectsWindowVisible & 1)) {
                this->_sceneObjectsWindowVisible++;
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
    ImVec2 pos = ImVec2(this->_window->width() - size.x - offset, this->_window->height() - size.y - offset);

    ImGui::SetNextWindowSize(size);
    ImGui::SetNextWindowPos(pos);

    if (ImGui::Begin("FPS", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                     ImGuiWindowFlags_NoCollapse)) {
        ImGui::Text("NOT WORKING");

        // TODO:
        // float delta = this->_engine->delta();
        // ImGui::Text("Frame time: %.3f ms", delta * 1000);
        // ImGui::Text("Frames per second: %.0f", 1.0f / delta);
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

            for (auto &[key, var]: this->_engineVars->vars()) {
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

            for (const auto &[id, resource]: this->_resourceManager->resources()) {
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

    if (ImGui::Begin("Scene tree")) {
        std::shared_ptr<Scene> currentScene = this->_sceneManager->currentScene();

        if (currentScene == nullptr) {
            ImGui::Text("Scene not available");
            ImGui::End();
            return;
        }

        // TODO:
//        if (ImGui::Button("Add object")) {
//            std::shared_ptr<MeshResource> cubeMesh = this->_engine->resourceManager()->loadMesh("cube");
//            std::shared_ptr<ImageResource> defaultImage = this->_engine->resourceManager()->loadDefaultImage();
//
//            Object *object = new Object();
//            object->mesh() = cubeMesh;
//            object->albedoTexture() = defaultImage;
//
//            this->_selectedObjectMeshName = this->_selectedObject->mesh()->id();
//            this->_selectedObjectAlbedoTextureName = this->_selectedObject->albedoTexture()->id();
//            this->_selectedObjectSpecularTextureName = this->_selectedObject->specTexture()->id();
//
//            currentScene->addObject(object);
//
//            this->_selectedObject = object;
//        }
//
//        ImGui::SameLine();
//
//        if (ImGui::Button("Remove all objects")) {
//            currentScene->clearObjects();
//            this->_selectedObject = nullptr;
//        }

        if (ImGui::BeginListBox("#scene_tree", ImVec2(-1, -1))) {
            std::function<void(const std::shared_ptr<SceneNode> &)> renderItem = [this, &renderItem](
                    const std::shared_ptr<SceneNode> &node) {
                const bool isSelected = this->_selectedSceneNode == node;

                ImGuiTreeNodeFlags flags = 0;

                if (isSelected) {
                    flags |= ImGuiTreeNodeFlags_Selected;
                }

                if (node->descendants().empty()) {
                    flags |= ImGuiTreeNodeFlags_Leaf;
                }

                if (ImGui::TreeNodeEx(node->displayName().c_str(), flags)) {
                    for (const auto &child: node->descendants()) {
                        renderItem(child);
                    }

                    if (ImGui::IsItemClicked()) {
                        // TODO: misclick
                        this->_selectedSceneNode = node;
                    }

                    ImGui::TreePop();
                }
            };

            renderItem(currentScene->root());

            ImGui::EndListBox();
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

            for (auto &[id, resource]: this->_resourceManager->resources()) {
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

DebugUI::DebugUI(const std::shared_ptr<EngineVars> &engineVars,
                 const std::shared_ptr<EventQueue> &eventQueue,
                 const std::shared_ptr<ResourceManager> &resourceManager,
                 const std::shared_ptr<SceneManager> &sceneManager,
                 const std::shared_ptr<Window> &window)
        : _engineVars(engineVars),
          _eventQueue(eventQueue),
          _resourceManager(resourceManager),
          _sceneManager(sceneManager),
          _window(window) {
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
    drawResourcesListWindow();
}
