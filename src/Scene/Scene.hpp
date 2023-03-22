#ifndef SCENE_SCENE_HPP
#define SCENE_SCENE_HPP

#include <memory>

#include "src/Scene/SceneIterator.hpp"

class SceneNode;

class Scene {
private:
    std::shared_ptr<SceneNode> _root;

public:
    [[nodiscard]] std::shared_ptr<SceneNode> &root() { return this->_root; }

    [[nodiscard]] SceneIterator iterate();

    static std::shared_ptr<Scene> empty();
};

#endif // SCENE_SCENE_HPP
