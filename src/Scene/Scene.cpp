#include "Scene.hpp"

#include <algorithm>

#include "src/Engine.hpp"
#include "src/Events/EventQueue.hpp"
#include "src/Scene/Light.hpp"
#include "src/Scene/Object.hpp"

Scene::Scene(Engine *engine, Skybox *skybox)
        : _engine(engine),
          _skybox(skybox) {
    //
}

Scene::~Scene() {
    this->clearObjects();
    this->clearLights();
}

void Scene::addObject(Object *object) {
    this->_objects.push_back(object);

    this->_engine->eventQueue()->pushEvent(Event{.type = OBJECT_CREATED_EVENT, .object = object});
}

void Scene::removeObject(Object *object) {
    this->_objects.erase(std::remove(this->_objects.begin(), this->_objects.end(), object));

    delete object;

    // TODO: try to avoid usage of pointer to deleted object
    this->_engine->eventQueue()->pushEvent(Event{.type = OBJECT_DESTROYED_EVENT, .object = object});
}

void Scene::addLight(Light *light) {
    this->_lights.push_back(light);

    this->_engine->eventQueue()->pushEvent(Event{.type = LIGHT_CREATED_EVENT, .light = light});
}

void Scene::removeLight(Light *light) {
    this->_lights.erase(std::remove(this->_lights.begin(), this->_lights.end(), light));

    delete light;

    // TODO: try to avoid usage of pointer to deleted object
    this->_engine->eventQueue()->pushEvent(Event{.type = LIGHT_DESTROYED_EVENT, .light = light});
}

void Scene::clearObjects() {
    for (Object *object: this->_objects) {
        delete object;

        // TODO: try to avoid usage of pointer to deleted object
        this->_engine->eventQueue()->pushEvent(Event{.type = OBJECT_DESTROYED_EVENT, .object = object});
    }

    this->_objects.clear();
}

void Scene::clearLights() {
    for (Light *light: this->_lights) {
        delete light;

        // TODO: try to avoid usage of pointer to deleted object
        this->_engine->eventQueue()->pushEvent(Event{.type = LIGHT_DESTROYED_EVENT, .light = light});
    }

    this->_lights.clear();
}
