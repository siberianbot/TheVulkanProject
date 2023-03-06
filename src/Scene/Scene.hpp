#ifndef SCENE_SCENE_HPP
#define SCENE_SCENE_HPP

#include <string>
#include <vector>

class Object;
class Skybox;
class Light;

class Scene {
private:
    std::vector<Object *> _objects;
    std::vector<Light *> _lights;
    Skybox *_skybox;

public:
    Scene(Skybox *skybox);
    ~Scene();

    [[nodiscard]] const std::vector<Object *> &objects() const { return this->_objects; }
    [[nodiscard]] const std::vector<Light *> &lights() const { return this->_lights; }
    [[nodiscard]] Skybox *skybox() const { return this->_skybox; }

    void addObject(Object *object);
    void removeObject(Object *object);

    void addLight(Light *light);
    void removeLight(Light *light);

    void clear();
};

#endif // SCENE_SCENE_HPP
