#ifndef SCENE_SCENE_HPP
#define SCENE_SCENE_HPP

#include <string>
#include <vector>

class Object;
class Skybox;

class Scene {
private:
    std::vector<Object *> _objects;
    Skybox *_skybox;

public:
    Scene(Skybox *skybox);
    ~Scene();

    [[nodiscard]] const std::vector<Object *> &objects() const { return this->_objects; }
    [[nodiscard]] Skybox *skybox() const { return this->_skybox; }

    void addObject(Object *object);
    void removeObject(Object *object);

    void clear();
};

#endif // SCENE_SCENE_HPP
