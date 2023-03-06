#include "Scene.hpp"

#include <algorithm>

#include "src/Scene/Light.hpp"
#include "src/Scene/Object.hpp"

Scene::Scene(Skybox *skybox)
        : _skybox(skybox) {
    //
}

Scene::~Scene() {
    this->clearObjects();
    this->clearLights();
}

void Scene::addObject(Object *object) {
    this->_objects.push_back(object);
}

void Scene::removeObject(Object *object) {
    this->_objects.erase(std::remove(this->_objects.begin(), this->_objects.end(), object));

    delete object;
}

void Scene::addLight(Light *light) {
    this->_lights.push_back(light);
}

void Scene::removeLight(Light *light) {
    this->_lights.erase(std::remove(this->_lights.begin(), this->_lights.end(), light));

    delete light;
}

void Scene::clearObjects() {
    for (Object *object: this->_objects) {
        delete object;
    }

    this->_objects.clear();
}

void Scene::clearLights() {
    for (Light *light: this->_lights) {
        delete light;
    }

    this->_lights.clear();
}
