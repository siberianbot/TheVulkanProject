#ifndef _OBJECT_HPP
#define _OBJECT_HPP

#include <glm/vec3.hpp>

class MeshResource;
class TextureResource;

// TODO
class Object {
public:
    MeshResource *mesh;
    TextureResource *texture;
    glm::vec3 pos;
};

#endif // _OBJECT_HPP
