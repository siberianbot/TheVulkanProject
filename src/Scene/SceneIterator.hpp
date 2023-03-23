#ifndef SRC_SCENE_SCENEITERATOR_HPP
#define SRC_SCENE_SCENEITERATOR_HPP

#include <memory>
#include <stack>
#include <vector>

class SceneNode;

class SceneIterator {
private:
    std::stack<std::shared_ptr<SceneNode>> _stack;
    std::vector<std::shared_ptr<SceneNode>> _discovered;

    bool isDiscovered(const std::shared_ptr<SceneNode> &node);

public:
    SceneIterator(const std::shared_ptr<SceneNode> &current);

    bool moveNext();

    const std::shared_ptr<SceneNode> &current() const;
};

#endif // SRC_SCENE_SCENEITERATOR_HPP
