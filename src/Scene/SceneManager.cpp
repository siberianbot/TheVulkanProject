#include "SceneManager.hpp"

#include "src/Events/EventQueue.hpp"
#include "src/Scene/Scene.hpp"

SceneManager::SceneManager(EventQueue *eventQueue)
        : _eventQueue(eventQueue) {
    //
}

void SceneManager::switchScene(Scene *newScene) {
    delete this->_currentScene;
    this->_currentScene = newScene;

    this->_eventQueue->pushEvent(Event{.type = SCENE_TRANSITION_EVENT, .scene = newScene});
}


