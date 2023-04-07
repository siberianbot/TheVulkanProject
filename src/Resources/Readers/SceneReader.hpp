#ifndef RESOURCES_READERS_SCENEREADER_HPP
#define RESOURCES_READERS_SCENEREADER_HPP

#include <memory>
#include <optional>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <nlohmann/json.hpp>

class Log;
class Object;
class Camera;
class LightSource;
class Prop;
class World;
class ModelComponent;
class PositionComponent;
class SkyboxComponent;
class ResourceData;
class SceneNode;

class SceneReader {
private:
    std::shared_ptr<Log> _log;

    glm::vec2 readVec2(const nlohmann::json &entry);
    glm::vec3 readVec3(const nlohmann::json &entry);

    std::shared_ptr<Object> readObjectEntry(const nlohmann::json &entry);

    std::shared_ptr<Camera> readCameraEntry(const nlohmann::json &entry);
    std::shared_ptr<LightSource> readLightSourceEntry(const nlohmann::json &entry);
    std::shared_ptr<Prop> readPropEntry(const nlohmann::json &entry);
    std::shared_ptr<World> readWorldEntry(const nlohmann::json &entry);

    std::shared_ptr<ModelComponent> readModelComponentEntry(const nlohmann::json &entry);
    std::shared_ptr<PositionComponent> readPositionComponentEntry(const nlohmann::json &entry);
    std::shared_ptr<SkyboxComponent> readSkyboxComponentEntry(const nlohmann::json &entry);

    std::shared_ptr<SceneNode> readEntry(const nlohmann::json &entry);
    std::shared_ptr<SceneNode> read(const std::weak_ptr<ResourceData> &resourceData);

public:
    SceneReader(const std::shared_ptr<Log> &log);

    [[nodiscard]] std::optional<std::shared_ptr<SceneNode>> tryRead(const std::weak_ptr<ResourceData> &resourceData);
};


#endif // RESOURCES_READERS_SCENEREADER_HPP
