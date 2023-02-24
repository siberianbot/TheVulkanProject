#include "Scene.hpp"

#include <algorithm>

#include "src/Scene/Object.hpp"

Scene::~Scene() {
    this->clear();
}

void Scene::addObject(Object *object) {
    this->_objects.push_back(object);
}

void Scene::removeObject(Object *object) {
    this->_objects.erase(std::remove(this->_objects.begin(), this->_objects.end(), object));

    delete object;
}

void Scene::clear() {
    for (Object *object: this->_objects) {
        delete object;
    }

    this->_objects.clear();
}
