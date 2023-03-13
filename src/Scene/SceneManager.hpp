#ifndef SCENE_SCENEMANAGER_HPP
#define SCENE_SCENEMANAGER_HPP

class EventQueue;
class Scene;

class SceneManager {
private:
    EventQueue *_eventQueue;

    Scene *_currentScene = nullptr;

public:
    SceneManager(EventQueue *eventQueue);

    void switchScene(Scene *newScene);

    [[nodiscard]] Scene *currentScene() { return this->_currentScene; }
};

#endif // SCENE_SCENEMANAGER_HPP
