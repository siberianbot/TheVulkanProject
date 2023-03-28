#include "ShaderCodeEditorWindow.hpp"

#include <imgui.h>

#include "src/Debug/Strings.hpp"
#include "src/Resources/ResourceManager.hpp"
#include "src/Resources/ShaderResource.hpp"

static constexpr const uint32_t BUFFER_SIZE = 64 * 1024;

void ShaderCodeEditorWindow::selectShader(const std::shared_ptr<ShaderResource> &resource) {
    if (!this->_selectedShaderResource.expired()) {
        this->_selectedShaderResource.lock()->unloadCode();
    }

    this->_selectedShaderResource = resource;
    this->_selectedShaderId = resource != nullptr ? resource->id() : NONE_ITEM;
    this->_selectedShaderCode.clear();

    if (resource == nullptr) {
        return;
    }

    try {
        resource->loadCode();
    } catch (std::runtime_error &error) {
        // TODO: notify
        this->selectShader(nullptr);
        return;
    }

    this->_selectedShaderCode.resize(resource->shaderCode().size() + BUFFER_SIZE);

    std::copy(resource->shaderCode().begin(),
              resource->shaderCode().end(),
              this->_selectedShaderCode.begin());
}

ShaderCodeEditorWindow::ShaderCodeEditorWindow(const std::shared_ptr<ResourceManager> &resourceManager)
        : _resourceManager(resourceManager) {
    //
}

void ShaderCodeEditorWindow::draw(bool *visible) {
    if (!ImGui::Begin(SHADER_CODE_EDITOR_WINDOW_TITLE, visible)) {
        ImGui::End();

        return;
    }

    if (ImGui::BeginCombo("##shader", this->_selectedShaderId.c_str())) {
        // None
        {
            const bool selected = this->_selectedShaderResource.expired();

            if (ImGui::Selectable(NONE_ITEM, selected)) {
                this->selectShader(nullptr);
            }

            if (selected) {
                ImGui::SetItemDefaultFocus();
            }
        }

        for (const auto &[id, resource]: this->_resourceManager->resources()) {
            if (resource->type() != SHADER_RESOURCE) {
                continue;
            }

            const bool selected = !this->_selectedShaderResource.expired() &&
                                  this->_selectedShaderResource.lock() == resource;

            if (ImGui::Selectable(id.c_str(), selected)) {
                this->selectShader(std::dynamic_pointer_cast<ShaderResource>(resource));
            }

            if (selected) {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    ImGui::SameLine();

    if (this->_selectedShaderResource.expired()) {
        ImGui::BeginDisabled();
    }

    if (ImGui::Button(SHADER_CODE_EDITOR_SAVE)) {
        try {
            this->_selectedShaderResource.lock()->saveCode(this->_selectedShaderCode);
        } catch (std::runtime_error &error) {
            // TODO: notify
        }
    }

    ImGui::SameLine();

    if (ImGui::Button(SHADER_CODE_EDITOR_BUILD)) {
        if (!this->_selectedShaderResource.lock()->build()) {
            // TODO: notify
        }
    }

    ImGui::InputTextMultiline("##shader-code", this->_selectedShaderCode.data(), this->_selectedShaderCode.size(),
                              ImVec2(-1, -1));

    if (this->_selectedShaderResource.expired()) {
        ImGui::EndDisabled();
    }

    ImGui::End();
}
