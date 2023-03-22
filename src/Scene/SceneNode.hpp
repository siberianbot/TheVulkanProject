#ifndef SCENE_SCENENODE_HPP
#define SCENE_SCENENODE_HPP

#include <list>
#include <memory>
#include <string>

class Object;

class SceneNode {
private:
    std::shared_ptr<Object> _object = nullptr;
    std::list<std::shared_ptr<SceneNode>> _descendants;

public:
    [[nodiscard]] std::shared_ptr<Object> &object() { return this->_object; }

    [[nodiscard]] std::string displayName();
    [[nodiscard]] const std::list<std::shared_ptr<SceneNode>> &descendants() const { return this->_descendants; }

    void insert(const std::shared_ptr<SceneNode> &node);
    void remove(const std::shared_ptr<SceneNode> &node);

    static std::shared_ptr<SceneNode> empty();
};

#endif // SCENE_SCENENODE_HPP
