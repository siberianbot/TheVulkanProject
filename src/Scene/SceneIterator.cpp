#include "SceneIterator.hpp"

#include <algorithm>

#include "src/Engine/EngineError.hpp"
#include "src/Scene/SceneNode.hpp"

bool SceneIterator::isDiscovered(const std::shared_ptr<SceneNode> &node) {
    return std::find(this->_discovered.begin(), this->_discovered.end(), node) != this->_discovered.end();
}

SceneIterator::SceneIterator(const std::shared_ptr<SceneNode> &current) {
    this->_stack.push(current);
}

bool SceneIterator::moveNext() {
    if (this->_stack.empty()) {
        return false;
    }

    std::shared_ptr<SceneNode> node = this->_stack.top();
    this->_stack.pop();

    if (!isDiscovered(node)) {
        for (auto it = node->descendants().rbegin(); it != node->descendants().rend(); it++) {
            this->_stack.push(*it);
        }

        this->_discovered.push_back(node);
    }

    return !this->_stack.empty();
}

const std::shared_ptr<SceneNode> &SceneIterator::current() const {
    if (this->_stack.empty()) {
        throw EngineError("No scene nodes available");
    }

    return this->_stack.top();
}
