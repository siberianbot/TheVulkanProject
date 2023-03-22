#ifndef SCENE_SCENEMANAGER_HPP
#define SCENE_SCENEMANAGER_HPP

#include <memory>

class EventQueue;
class Scene;
class SceneNode;
class Object;

class SceneManager {
private:
    std::shared_ptr<EventQueue> _eventQueue;

    std::shared_ptr<Scene> _currentScene = nullptr;

public:
    SceneManager(const std::shared_ptr<EventQueue> &eventQueue);

    void setScene(const std::shared_ptr<Scene> &scene);

    std::shared_ptr<SceneNode> addObject(const std::shared_ptr<Object> &object);
    void removeNode(const std::shared_ptr<SceneNode> &node);

    [[nodiscard]] std::shared_ptr<Scene> currentScene() { return this->_currentScene; }
};

#endif // SCENE_SCENEMANAGER_HPP
