#include "SceneNode.hpp"

#include <algorithm>

#include "src/Objects/Object.hpp"

std::string SceneNode::displayName() {
    return this->_object != nullptr
           ? this->_object->displayName()
           : "empty node";
}

void SceneNode::insert(const std::shared_ptr<SceneNode> &node) {
    this->_descendants.push_back(node);
}

void SceneNode::remove(const std::shared_ptr<SceneNode> &node) {
    auto it = std::find(this->_descendants.begin(), this->_descendants.end(), node);

    if (it != this->_descendants.end()) {
        this->_descendants.erase(it);
    } else {
        for (const auto &item: this->_descendants) {
            item->remove(node);
        }
    }
}

std::shared_ptr<SceneNode> SceneNode::empty() {
    return std::make_shared<SceneNode>();
}
