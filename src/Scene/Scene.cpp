#include "Scene.hpp"

#include "src/Scene/SceneNode.hpp"

std::shared_ptr<Scene> Scene::empty() {
    std::shared_ptr<Scene> scene = std::make_shared<Scene>();

    scene->_root = SceneNode::empty();

    return scene;
}

SceneIterator Scene::iterate() {
    return SceneIterator(this->_root);
}
