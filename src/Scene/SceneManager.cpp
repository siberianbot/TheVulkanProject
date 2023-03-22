#include "SceneManager.hpp"

#include "src/Events/EventQueue.hpp"
#include "src/Scene/Scene.hpp"
#include "src/Scene/SceneNode.hpp"

SceneManager::SceneManager(const std::shared_ptr<EventQueue> &eventQueue)
        : _eventQueue(eventQueue) {
    //
}

void SceneManager::setScene(const std::shared_ptr<Scene> &scene) {
    this->_currentScene = scene;
    this->_eventQueue->pushEvent(Event{.type = TRANSITION_SCENE_EVENT, .value = scene});
}

std::shared_ptr<SceneNode> SceneManager::addObject(const std::shared_ptr<Object> &object) {
    std::shared_ptr<SceneNode> node = std::make_shared<SceneNode>();
    node->object() = object;

    this->_currentScene->root()->insert(node);
    this->_eventQueue->pushEvent(Event{.type = CREATED_OBJECT_EVENT, .value = object});

    return node;
}

void SceneManager::removeNode(const std::shared_ptr<SceneNode> &node) {
    this->_currentScene->root()->remove(node);
    this->_eventQueue->pushEvent(Event{.type = DESTROYED_OBJECT_EVENT, .value = node->object()});
}
