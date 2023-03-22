#include "SceneIterator.hpp"

#include <algorithm>

#include "src/Scene/SceneNode.hpp"

bool SceneIterator::isDiscovered(const std::shared_ptr<SceneNode> &node) {
    return std::find(this->_discovered.begin(), this->_discovered.end(), node) != this->_discovered.end();
}

SceneIterator::SceneIterator(const std::shared_ptr<SceneNode> &current)
        : _current(current) {
    this->_stack.push(current);
}

bool SceneIterator::moveNext() {
    if (this->_stack.empty()) {
        this->_current = nullptr;
        return false;
    }

    std::shared_ptr<SceneNode> node = this->_stack.top();
    this->_stack.pop();

    if (!isDiscovered(node)) {
        for (const auto &child: node->descendants()) {
            this->_stack.push(child);
        }
    }

    return false;
}
