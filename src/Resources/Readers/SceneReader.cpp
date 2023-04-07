#include "SceneReader.hpp"

#include <fmt/core.h>

#include "src/Engine/EngineError.hpp"
#include "src/Engine/Log.hpp"
#include "src/Objects/Camera.hpp"
#include "src/Objects/LightSource.hpp"
#include "src/Objects/Prop.hpp"
#include "src/Objects/World.hpp"
#include "src/Objects/Components/ModelComponent.hpp"
#include "src/Objects/Components/PositionComponent.hpp"
#include "src/Objects/Components/SkyboxComponent.hpp"
#include "src/Resources/ResourceData.hpp"
#include "src/Scene/SceneNode.hpp"

static constexpr const char *SCENE_READER_TAG = "SceneReader";

static constexpr const char *SCENE_ENTRY_OBJECT_TAG = "object";
static constexpr const char *SCENE_ENTRY_DESCENDANTS_TAG = "descendants";

static constexpr const char *OBJECT_ENTRY_CLASS_TAG = "class";
static constexpr const char *OBJECT_ENTRY_CLASS_CAMERA = "camera";
static constexpr const char *OBJECT_ENTRY_CLASS_LIGHTSOURCE = "light-source";
static constexpr const char *OBJECT_ENTRY_CLASS_PROP = "prop";
static constexpr const char *OBJECT_ENTRY_CLASS_WORLD = "world";

static constexpr const char *CAMERA_ENTRY_NEAR_TAG = "near";
static constexpr const char *CAMERA_ENTRY_FAR_TAG = "far";
static constexpr const char *CAMERA_ENTRY_FOV_TAG = "fov";

static constexpr const char *LIGHT_SOURCE_ENTRY_TYPE_TAG = "type";
static constexpr const char *LIGHT_SOURCE_ENTRY_ENABLED_TAG = "enabled";
static constexpr const char *LIGHT_SOURCE_ENTRY_COLOR_TAG = "color";
static constexpr const char *LIGHT_SOURCE_ENTRY_RANGE_TAG = "range";
static constexpr const char *LIGHT_SOURCE_ENTRY_RECT_TAG = "rect";
static constexpr const char *LIGHT_SOURCE_ENTRY_ANGLE_TAG = "angle";

static constexpr const char *OBJECT_ENTRY_COMPONENTS_TAG = "components";
static constexpr const char *COMPONENTS_ENTRY_MODEL_TAG = "model";
static constexpr const char *COMPONENTS_ENTRY_POSITION_TAG = "position";
static constexpr const char *COMPONENTS_ENTRY_SKYBOX_TAG = "skybox";

static constexpr const char *MODEL_COMPONENT_MESH_TAG = "mesh";
static constexpr const char *MODEL_COMPONENT_ALBEDO_TEXTURE_TAG = "albedoTexture";
static constexpr const char *MODEL_COMPONENT_SPECULAR_TEXTURE_TAG = "specularTexture";

static constexpr const char *POSITION_COMPONENT_POSITION_TAG = "position";
static constexpr const char *POSITION_COMPONENT_ROTATION_TAG = "rotation";
static constexpr const char *POSITION_COMPONENT_SCALE_TAG = "scale";

static constexpr const char *SKYBOX_COMPONENT_MESH_TAG = "mesh";
static constexpr const char *SKYBOX_COMPONENT_TEXTURES_TAG = "textures";

glm::vec2 SceneReader::readVec2(const nlohmann::json &entry) {
    const uint32_t VECTOR2_SIZE = 2;

    if (!entry.is_array() || entry.size() != VECTOR2_SIZE) {
        throw EngineError(fmt::format("Vector2 entry must be a number array of size {0}",
                                      VECTOR2_SIZE));
    }

    glm::vec2 vec;

    for (uint32_t idx = 0; idx < VECTOR2_SIZE; idx++) {
        if (!entry[idx].is_number()) {
            throw EngineError(fmt::format("Item #{0} of vector2 entry must be a number", idx));
        }

        vec[idx] = entry[idx];
    }

    return vec;
}

glm::vec3 SceneReader::readVec3(const nlohmann::json &entry) {
    const uint32_t VECTOR3_SIZE = 3;

    if (!entry.is_array() || entry.size() != VECTOR3_SIZE) {
        throw EngineError(fmt::format("Vector3 entry must be a number array of size {0}",
                                      VECTOR3_SIZE));
    }

    glm::vec3 vec;

    for (uint32_t idx = 0; idx < VECTOR3_SIZE; idx++) {
        if (!entry[idx].is_number()) {
            throw EngineError(fmt::format("Item #{0} of vector3 entry must be a number", idx));
        }

        vec[idx] = entry[idx];
    }

    return vec;
}

std::shared_ptr<Object> SceneReader::readObjectEntry(const nlohmann::json &entry) {
    if (!entry.is_object()) {
        throw EngineError("Object entry must be an object");
    }

    if (!entry.contains(OBJECT_ENTRY_CLASS_TAG)) {
        throw EngineError("Object entry does not contain class");
    }

    std::string type = entry[OBJECT_ENTRY_CLASS_TAG];

    if (type == OBJECT_ENTRY_CLASS_CAMERA) {
        return this->readCameraEntry(entry);
    }

    if (type == OBJECT_ENTRY_CLASS_LIGHTSOURCE) {
        return this->readLightSourceEntry(entry);
    }

    if (type == OBJECT_ENTRY_CLASS_PROP) {
        return this->readPropEntry(entry);
    }

    if (type == OBJECT_ENTRY_CLASS_WORLD) {
        return this->readWorldEntry(entry);
    }

    throw EngineError(fmt::format("Object entry have unknown class {0}", type));
}

std::shared_ptr<Camera> SceneReader::readCameraEntry(const nlohmann::json &entry) {
    if (!entry.is_object()) {
        throw EngineError("Camera object entry must be an object");
    }

    std::shared_ptr<PositionComponent> position = nullptr;

    if (entry.contains(OBJECT_ENTRY_COMPONENTS_TAG)) {
        if (!entry[OBJECT_ENTRY_COMPONENTS_TAG].is_object()) {
            throw EngineError(fmt::format("Tag {0} of camera source object entry must be an object",
                                          OBJECT_ENTRY_COMPONENTS_TAG));
        }

        if (entry[OBJECT_ENTRY_COMPONENTS_TAG].contains(COMPONENTS_ENTRY_POSITION_TAG)) {
            position = this->readPositionComponentEntry(
                    entry[OBJECT_ENTRY_COMPONENTS_TAG][COMPONENTS_ENTRY_POSITION_TAG]);
        }
    }

    if (position == nullptr) {
        position = std::make_shared<PositionComponent>();
    }

    std::shared_ptr<Camera> camera = std::make_shared<Camera>(position);

    if (entry.contains(CAMERA_ENTRY_NEAR_TAG)) {
        if (!entry[CAMERA_ENTRY_NEAR_TAG].is_number()) {
            throw EngineError(fmt::format("Tag {0} of camera object entry must be a number",
                                          CAMERA_ENTRY_NEAR_TAG));
        }

        camera->near() = entry[CAMERA_ENTRY_NEAR_TAG];
    }

    if (entry.contains(CAMERA_ENTRY_FAR_TAG)) {
        if (!entry[CAMERA_ENTRY_FAR_TAG].is_number()) {
            throw EngineError(fmt::format("Tag {0} of camera object entry must be a number",
                                          CAMERA_ENTRY_FAR_TAG));
        }

        camera->far() = entry[CAMERA_ENTRY_FAR_TAG];
    }

    if (entry.contains(CAMERA_ENTRY_FOV_TAG)) {
        if (!entry[CAMERA_ENTRY_FOV_TAG].is_number()) {
            throw EngineError(fmt::format("Tag {0} of camera object entry must be a number",
                                          CAMERA_ENTRY_FOV_TAG));
        }

        camera->fov() = glm::radians((float) entry[CAMERA_ENTRY_FOV_TAG]);
    }

    return camera;
}

std::shared_ptr<LightSource> SceneReader::readLightSourceEntry(const nlohmann::json &entry) {
    if (!entry.is_object()) {
        throw EngineError("Light source object entry must be an object");
    }

    std::shared_ptr<PositionComponent> position = nullptr;

    if (entry.contains(OBJECT_ENTRY_COMPONENTS_TAG)) {
        if (!entry[OBJECT_ENTRY_COMPONENTS_TAG].is_object()) {
            throw EngineError(fmt::format("Tag {0} of light source object entry must be an object",
                                          OBJECT_ENTRY_COMPONENTS_TAG));
        }

        if (entry[OBJECT_ENTRY_COMPONENTS_TAG].contains(COMPONENTS_ENTRY_POSITION_TAG)) {
            position = this->readPositionComponentEntry(
                    entry[OBJECT_ENTRY_COMPONENTS_TAG][COMPONENTS_ENTRY_POSITION_TAG]);
        }
    }

    if (position == nullptr) {
        position = std::make_shared<PositionComponent>();
    }

    std::shared_ptr<LightSource> lightSource = std::make_shared<LightSource>(position);

    if (entry.contains(LIGHT_SOURCE_ENTRY_TYPE_TAG)) {
        if (!entry[LIGHT_SOURCE_ENTRY_TYPE_TAG].is_string()) {
            throw EngineError(fmt::format("Tag {0} of light source object entry must be a string",
                                          LIGHT_SOURCE_ENTRY_TYPE_TAG));
        }

        lightSource->type() = lightSourceTypeFromString(entry[LIGHT_SOURCE_ENTRY_TYPE_TAG]);
    }

    if (entry.contains(LIGHT_SOURCE_ENTRY_ENABLED_TAG)) {
        if (!entry[LIGHT_SOURCE_ENTRY_ENABLED_TAG].is_boolean()) {
            throw EngineError(fmt::format("Tag {0} of light source object entry must be a boolean",
                                          LIGHT_SOURCE_ENTRY_ENABLED_TAG));
        }

        lightSource->enabled() = entry[LIGHT_SOURCE_ENTRY_ENABLED_TAG];
    }

    if (entry.contains(LIGHT_SOURCE_ENTRY_RANGE_TAG)) {
        if (!entry[LIGHT_SOURCE_ENTRY_RANGE_TAG].is_number()) {
            throw EngineError(fmt::format("Tag {0} of light source object entry must be a number",
                                          LIGHT_SOURCE_ENTRY_RANGE_TAG));
        }

        lightSource->range() = entry[LIGHT_SOURCE_ENTRY_RANGE_TAG];
    }

    if (entry.contains(LIGHT_SOURCE_ENTRY_ANGLE_TAG)) {
        if (!entry[LIGHT_SOURCE_ENTRY_ANGLE_TAG].is_number()) {
            throw EngineError(fmt::format("Tag {0} of light source object entry must be a number",
                                          LIGHT_SOURCE_ENTRY_ANGLE_TAG));
        }

        lightSource->angle() = glm::radians((float) entry[LIGHT_SOURCE_ENTRY_ANGLE_TAG]);
    }

    if (entry.contains(LIGHT_SOURCE_ENTRY_RECT_TAG)) {
        lightSource->rect() = this->readVec2(entry[LIGHT_SOURCE_ENTRY_RECT_TAG]);
    }

    if (entry.contains(LIGHT_SOURCE_ENTRY_COLOR_TAG)) {
        lightSource->color() = this->readVec3(entry[LIGHT_SOURCE_ENTRY_COLOR_TAG]);
    }

    return lightSource;
}

std::shared_ptr<Prop> SceneReader::readPropEntry(const nlohmann::json &entry) {
    if (!entry.is_object()) {
        throw EngineError("Prop object entry must be an object");
    }

    std::shared_ptr<PositionComponent> position = nullptr;
    std::shared_ptr<ModelComponent> model = nullptr;

    if (entry.contains(OBJECT_ENTRY_COMPONENTS_TAG)) {
        if (!entry[OBJECT_ENTRY_COMPONENTS_TAG].is_object()) {
            throw EngineError(fmt::format("Tag {0} of prop object entry must be an object",
                                          OBJECT_ENTRY_COMPONENTS_TAG));
        }

        if (entry[OBJECT_ENTRY_COMPONENTS_TAG].contains(COMPONENTS_ENTRY_POSITION_TAG)) {
            position = this->readPositionComponentEntry(
                    entry[OBJECT_ENTRY_COMPONENTS_TAG][COMPONENTS_ENTRY_POSITION_TAG]);
        }

        if (entry[OBJECT_ENTRY_COMPONENTS_TAG].contains(COMPONENTS_ENTRY_MODEL_TAG)) {
            model = this->readModelComponentEntry(entry[OBJECT_ENTRY_COMPONENTS_TAG][COMPONENTS_ENTRY_MODEL_TAG]);
        }
    }

    if (position == nullptr) {
        position = std::make_shared<PositionComponent>();
    }

    if (model == nullptr) {
        model = std::make_shared<ModelComponent>();
    }

    return std::make_shared<Prop>(position, model);
}

std::shared_ptr<World> SceneReader::readWorldEntry(const nlohmann::json &entry) {
    if (!entry.is_object()) {
        throw EngineError("World object entry must be an object");
    }

    std::shared_ptr<SkyboxComponent> skybox = nullptr;

    if (entry.contains(OBJECT_ENTRY_COMPONENTS_TAG)) {
        if (!entry[OBJECT_ENTRY_COMPONENTS_TAG].is_object()) {
            throw EngineError(fmt::format("Tag {0} of world object entry must be an object",
                                          OBJECT_ENTRY_COMPONENTS_TAG));
        }

        if (entry[OBJECT_ENTRY_COMPONENTS_TAG].contains(COMPONENTS_ENTRY_SKYBOX_TAG)) {
            skybox = this->readSkyboxComponentEntry(entry[OBJECT_ENTRY_COMPONENTS_TAG][COMPONENTS_ENTRY_SKYBOX_TAG]);
        }
    }

    if (skybox == nullptr) {
        skybox = std::make_shared<SkyboxComponent>();
    }

    return std::make_shared<World>(skybox);
}

std::shared_ptr<ModelComponent> SceneReader::readModelComponentEntry(const nlohmann::json &entry) {
    if (!entry.is_object()) {
        throw EngineError("Model component entry must be an object");
    }

    std::shared_ptr<ModelComponent> component = std::make_shared<ModelComponent>();

    if (entry.contains(MODEL_COMPONENT_MESH_TAG)) {
        if (!entry[MODEL_COMPONENT_MESH_TAG].is_string()) {
            throw EngineError(fmt::format("Tag {0} of model component entry must be a string",
                                          MODEL_COMPONENT_MESH_TAG));
        }

        component->setMeshId(entry[MODEL_COMPONENT_MESH_TAG]);
    }

    if (entry.contains(MODEL_COMPONENT_ALBEDO_TEXTURE_TAG)) {
        if (!entry[MODEL_COMPONENT_ALBEDO_TEXTURE_TAG].is_string()) {
            throw EngineError(fmt::format("Tag {0} of model component entry must be a string",
                                          MODEL_COMPONENT_ALBEDO_TEXTURE_TAG));
        }

        component->setAlbedoTextureId(entry[MODEL_COMPONENT_ALBEDO_TEXTURE_TAG]);
    }

    if (entry.contains(MODEL_COMPONENT_SPECULAR_TEXTURE_TAG)) {
        if (!entry[MODEL_COMPONENT_SPECULAR_TEXTURE_TAG].is_string()) {
            throw EngineError(fmt::format("Tag {0} of model component entry must be a string",
                                          MODEL_COMPONENT_SPECULAR_TEXTURE_TAG));
        }

        component->setSpecularTextureId(entry[MODEL_COMPONENT_SPECULAR_TEXTURE_TAG]);
    }

    return component;
}

std::shared_ptr<PositionComponent> SceneReader::readPositionComponentEntry(const nlohmann::json &entry) {
    if (!entry.is_object()) {
        throw EngineError("Position component entry must be an object");
    }

    std::shared_ptr<PositionComponent> component = std::make_shared<PositionComponent>();

    if (entry.contains(POSITION_COMPONENT_POSITION_TAG)) {
        component->position() = this->readVec3(entry[POSITION_COMPONENT_POSITION_TAG]);
    }

    if (entry.contains(POSITION_COMPONENT_ROTATION_TAG)) {
        component->rotation() = this->readVec3(entry[POSITION_COMPONENT_ROTATION_TAG]);
    }

    if (entry.contains(POSITION_COMPONENT_SCALE_TAG)) {
        component->scale() = this->readVec3(entry[POSITION_COMPONENT_SCALE_TAG]);
    }

    return component;
}

std::shared_ptr<SkyboxComponent> SceneReader::readSkyboxComponentEntry(const nlohmann::json &entry) {
    if (!entry.is_object()) {
        throw EngineError("Skybox component entry must be an object");
    }

    std::shared_ptr<SkyboxComponent> component = std::make_shared<SkyboxComponent>();

    if (entry.contains(SKYBOX_COMPONENT_MESH_TAG)) {
        if (!entry[SKYBOX_COMPONENT_MESH_TAG].is_string()) {
            throw EngineError(fmt::format("Tag {0} of skybox component entry must be a string",
                                          SKYBOX_COMPONENT_MESH_TAG));
        }

        component->setMeshId(entry[SKYBOX_COMPONENT_MESH_TAG]);
    }

    if (entry.contains(SKYBOX_COMPONENT_TEXTURES_TAG)) {
        if (!entry[SKYBOX_COMPONENT_TEXTURES_TAG].is_array() ||
            entry[SKYBOX_COMPONENT_TEXTURES_TAG].size() != SKYBOX_TEXTURE_ARRAY_SIZE) {
            throw EngineError(fmt::format("Tag {0} of skybox component entry must be a string array of size {1}",
                                          SKYBOX_COMPONENT_TEXTURES_TAG, SKYBOX_TEXTURE_ARRAY_SIZE));
        }

        std::array<ResourceId, SKYBOX_TEXTURE_ARRAY_SIZE> textureIds;

        for (uint32_t idx = 0; idx < SKYBOX_TEXTURE_ARRAY_SIZE; idx++) {
            nlohmann::json textureEntry = entry[SKYBOX_COMPONENT_TEXTURES_TAG][idx];

            if (!textureEntry.is_string()) {
                throw EngineError(fmt::format("Item #{0} of tag {1} in skybox component entry must be a string",
                                              idx, SKYBOX_COMPONENT_TEXTURES_TAG));
            }

            textureIds[idx] = textureEntry;
        }

        component->setTextureIds(textureIds);
    }

    return component;
}

std::shared_ptr<SceneNode> SceneReader::readEntry(const nlohmann::json &entry) {
    if (!entry.is_object()) {
        throw EngineError("Scene node entry must be an object");
    }

    std::shared_ptr<SceneNode> node = std::make_shared<SceneNode>();

    if (entry.contains(SCENE_ENTRY_OBJECT_TAG)) {
        try {
            node->object() = this->readObjectEntry(entry[SCENE_ENTRY_OBJECT_TAG]);
        } catch (const std::exception &error) {
            node->object() = nullptr;
            this->_log->warning(SCENE_READER_TAG, error);
        }
    }

    if (entry.contains(SCENE_ENTRY_DESCENDANTS_TAG)) {
        if (!entry[SCENE_ENTRY_DESCENDANTS_TAG].is_array()) {
            throw EngineError(fmt::format("Tag {0} of scene node entry must be an array", SCENE_ENTRY_DESCENDANTS_TAG));
        }

        for (const nlohmann::json &subEntry: entry[SCENE_ENTRY_DESCENDANTS_TAG]) {
            try {
                node->insert(this->readEntry(subEntry));
            } catch (const std::exception &error) {
                this->_log->warning(SCENE_READER_TAG, error);
            }
        }
    }

    return node;
}

std::shared_ptr<SceneNode> SceneReader::read(const std::weak_ptr<ResourceData> &resourceData) {
    std::shared_ptr<ResourceData> lockedResourceData = resourceData.lock();

    return this->readEntry(nlohmann::json::parse(lockedResourceData->data(), nullptr, true, true));
}

SceneReader::SceneReader(const std::shared_ptr<Log> &log)
        : _log(log) {
    //
}

std::optional<std::shared_ptr<SceneNode>> SceneReader::tryRead(const std::weak_ptr<ResourceData> &resourceData) {
    try {
        return this->read(resourceData);
    } catch (const std::exception &error) {
        this->_log->error(SCENE_READER_TAG, error);
        return std::nullopt;
    }
}
