#include "ShaderCodeEditorWindow.hpp"

#include <imgui.h>

#include "src/Debug/Strings.hpp"
#include "src/Resources/Resource.hpp"
#include "src/Resources/ResourceDatabase.hpp"

static constexpr const uint32_t BUFFER_SIZE = 64 * 1024;

void ShaderCodeEditorWindow::selectShader(const std::shared_ptr<Resource> &resource) {
//    if (!this->_selectedShaderResource.expired()) {
//        this->_selectedShaderResource.lock()->unloadCode();
//    }
//
//    this->_selectedShaderResource = resource;
//    this->_selectedShaderId = resource != nullptr ? resource->id() : NONE_ITEM;
//    this->_selectedShaderCode.clear();
//
//    if (resource == nullptr) {
//        return;
//    }
//
//    try {
//        resource->loadCode();
//    } catch (std::runtime_error &error) {
//        // TODO: notify
//        this->selectShader(nullptr);
//        return;
//    }
//
//    this->_selectedShaderCode.resize(resource->shaderCode().size() + BUFFER_SIZE);
//
//    std::copy(resource->shaderCode().begin(),
//              resource->shaderCode().end(),
//              this->_selectedShaderCode.begin());
}

ShaderCodeEditorWindow::ShaderCodeEditorWindow(const std::shared_ptr<ResourceDatabase> &resourceDatabase)
        : _resourceDatabase(resourceDatabase) {
    //
}

void ShaderCodeEditorWindow::draw(bool *visible) {
    if (!ImGui::Begin(SHADER_CODE_EDITOR_WINDOW_TITLE, visible)) {
        ImGui::End();

        return;
    }

    if (ImGui::BeginCombo("##shader", this->_selectedId.c_str())) {
        // None
        {
            const bool selected = this->_selectedResource.expired();

            if (ImGui::Selectable(NONE_ITEM, selected)) {
                this->selectShader(nullptr);
            }

            if (selected) {
                ImGui::SetItemDefaultFocus();
            }
        }

        for (const auto &[id, resource]: this->_resourceDatabase->resources()) {
            if (resource->type() != SHADER_RESOURCE) {
                continue;
            }

            const bool selected = !this->_selectedResource.expired() &&
                                  this->_selectedResource.lock() == resource;

            if (ImGui::Selectable(id.c_str(), selected)) {
                this->selectShader(resource);
            }

            if (selected) {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    ImGui::SameLine();

    if (this->_selectedResource.expired()) {
        ImGui::BeginDisabled();
    }

    if (ImGui::Button(SHADER_CODE_EDITOR_SAVE)) {
        try {
//            TODO: this->_selectedShaderResource.lock()->saveCode(this->_selectedShaderCode);
        } catch (std::runtime_error &error) {
            // TODO: notify
        }
    }

    ImGui::SameLine();

    if (ImGui::Button(SHADER_CODE_EDITOR_BUILD)) {
        // TODO:
//        if (!this->_selectedShaderResource.lock()->build()) {
//            // TODO: notify
//        }
    }

    ImGui::Separator();

    ImGui::InputTextMultiline("##shader-code", this->_code.data(), this->_code.size(),
                              ImVec2(-1, -1));

    if (this->_selectedResource.expired()) {
        ImGui::EndDisabled();
    }

    ImGui::End();
}
