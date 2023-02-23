#ifndef _OBJECT_HPP
#define _OBJECT_HPP

#include <glm/vec3.hpp>

class MeshRenderingResource;
class TextureRenderingResource;

// TODO
class Object {
public:
    MeshRenderingResource *mesh;
    TextureRenderingResource *texture;
    glm::vec3 pos;
};

#endif // _OBJECT_HPP
