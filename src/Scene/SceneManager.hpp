#ifndef SCENE_SCENEMANAGER_HPP
#define SCENE_SCENEMANAGER_HPP

#include <memory>

class EventQueue;
class Scene;
class SceneNode;
class Object;
class Camera;

class SceneManager {
private:
    std::shared_ptr<EventQueue> _eventQueue;

    std::shared_ptr<Scene> _currentScene = nullptr;
    std::weak_ptr<Camera> _currentCamera;

public:
    SceneManager(const std::shared_ptr<EventQueue> &eventQueue);

    void setScene(const std::shared_ptr<Scene> &scene);

    std::shared_ptr<SceneNode> addObject(const std::shared_ptr<Object> &object);
    void removeNode(const std::shared_ptr<SceneNode> &node);

    [[nodiscard]] std::shared_ptr<Scene> currentScene() { return this->_currentScene; }
    [[nodiscard]] std::weak_ptr<Camera> &currentCamera() { return this->_currentCamera; }
};

#endif // SCENE_SCENEMANAGER_HPP
