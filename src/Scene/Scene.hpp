#ifndef SCENE_SCENE_HPP
#define SCENE_SCENE_HPP

#include <string>
#include <vector>

class Object;

class Scene {
private:
    std::vector<Object *> _objects;

public:
    ~Scene();

    [[nodiscard]] const std::vector<Object *> &objects() const { return this->_objects; }

    void addObject(Object *object);
    void removeObject(Object *object);

    void clear();
};

#endif // SCENE_SCENE_HPP
