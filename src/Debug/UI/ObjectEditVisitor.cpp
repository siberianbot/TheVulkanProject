#include "ObjectEditVisitor.hpp"

#include <functional>
#include <map>
#include <type_traits>

#include <imgui.h>

#include "src/Debug/Strings.hpp"
#include "src/Objects/Camera.hpp"
#include "src/Objects/LightSource.hpp"
#include "src/Objects/Components/ModelComponent.hpp"
#include "src/Objects/Components/PositionComponent.hpp"
#include "src/Objects/Components/SkyboxComponent.hpp"
#include "src/Resources/CubeImageResource.hpp"
#include "src/Resources/MeshResource.hpp"
#include "src/Resources/ImageResource.hpp"
#include "src/Resources/ResourceManager.hpp"

static std::map<LightSourceType, std::string> LIGHT_SOURCE_TYPES = {
        {POINT_LIGHT_SOURCE,     toString(POINT_LIGHT_SOURCE)},
        {CONE_LIGHT_SOURCE,      toString(CONE_LIGHT_SOURCE)},
        {RECTANGLE_LIGHT_SOURCE, toString(RECTANGLE_LIGHT_SOURCE)}
};

template<typename T>
void drawResourceSelect(const std::shared_ptr<ResourceManager> &resourceManager,
                        const std::string &title, ResourceType type, const std::weak_ptr<T> &resourceRef,
                        const std::function<void(const std::shared_ptr<T> &)> &callback) {
    static_assert(std::is_base_of<Resource, T>::value);

    if (ImGui::BeginCombo(title.c_str(), !resourceRef.expired() ? resourceRef.lock()->id().c_str() : NONE_ITEM)) {
        {
            const bool selected = resourceRef.expired();

            if (ImGui::Selectable(NONE_ITEM, selected)) {
                callback(nullptr);
            }

            if (selected) {
                ImGui::SetItemDefaultFocus();
            }
        }

        for (const auto &[id, resource]: resourceManager->resources()) {
            if (resource->type() != type) {
                continue;
            }

            const bool selected = !resourceRef.expired() && resourceRef.lock() == resource;

            if (ImGui::Selectable(id.c_str(), selected)) {
                callback(std::dynamic_pointer_cast<T>(resource));
            }

            if (selected) {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

}

ObjectEditVisitor::ObjectEditVisitor(const std::shared_ptr<ResourceManager> &resourceManager)
        : _resourceManager(resourceManager) {
    //
}

void ObjectEditVisitor::drawCameraObject(Camera *camera) {
    ImGui::InputFloat("Near Plane", &camera->near());
    ImGui::InputFloat("Far Plane", &camera->far());
    ImGui::SliderFloat("Field of View", &camera->fov(), 0, glm::radians(180.0f));
}

void ObjectEditVisitor::drawLightSourceObject(LightSource *lightSource) {
    ImGui::Checkbox("Enabled", &lightSource->enabled());

    if (ImGui::BeginCombo("Type", toString(lightSource->type()).c_str())) {
        for (const auto &[type, name]: LIGHT_SOURCE_TYPES) {
            const bool isSelected = lightSource->type() == type;

            if (ImGui::Selectable(name.c_str(), isSelected)) {
                lightSource->type() = type;
            }

            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    ImGui::ColorPicker3("Color", reinterpret_cast<float *>(&lightSource->color()));
    ImGui::InputFloat("Range", &lightSource->range());

    switch (lightSource->type()) {
        case POINT_LIGHT_SOURCE:
        case CONE_LIGHT_SOURCE:
            ImGui::SliderFloat("Angle", &lightSource->angle(), 0, glm::radians(180.0f));
            break;

        case RECTANGLE_LIGHT_SOURCE:
            ImGui::InputScalarN("Rectangle", ImGuiDataType_Float, &lightSource->rect(), 2);
            break;
    }
}

void ObjectEditVisitor::drawModelComponent(ModelComponent *component) {
    ImGui::Text("Model Component");

    drawResourceSelect<MeshResource>(this->_resourceManager, "Mesh", MESH_RESOURCE, component->mesh(),
                                     [&component](const std::shared_ptr<MeshResource> &mesh) {
                                         component->setMesh(mesh);
                                     });

    drawResourceSelect<ImageResource>(this->_resourceManager, "Albedo Texture", IMAGE_RESOURCE,
                                      component->albedoTexture(),
                                      [&component](const std::shared_ptr<ImageResource> &texture) {
                                          component->setAlbedoTexture(texture);
                                      });

    drawResourceSelect<ImageResource>(this->_resourceManager, "Specular Texture", IMAGE_RESOURCE,
                                      component->specularTexture(),
                                      [&component](const std::shared_ptr<ImageResource> &texture) {
                                          component->setSpecularTexture(texture);
                                      });
}

void ObjectEditVisitor::drawPositionComponent(PositionComponent *component) {
    ImGui::Text("Position Component");

    ImGui::InputScalarN("Position", ImGuiDataType_Float, &component->position(), 3);
    ImGui::SliderFloat3("Rotation", reinterpret_cast<float *>(&component->rotation()), 0, glm::radians(360.0f));
    ImGui::InputScalarN("Scale", ImGuiDataType_Float, &component->scale(), 3);
}

void ObjectEditVisitor::drawSkyboxComponent(SkyboxComponent *component) {
    ImGui::Text("Skybox Component");

    drawResourceSelect<MeshResource>(this->_resourceManager, "Mesh", MESH_RESOURCE, component->mesh(),
                                     [&component](const std::shared_ptr<MeshResource> &mesh) {
                                         component->setMesh(mesh);
                                     });

    drawResourceSelect<CubeImageResource>(this->_resourceManager, "Texture", CUBE_IMAGE_RESOURCE,
                                          component->texture(),
                                          [&component](const std::shared_ptr<CubeImageResource> &texture) {
                                              component->setTexture(texture);
                                          });
}
