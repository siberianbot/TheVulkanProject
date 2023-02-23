#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

struct MeshRenderingResource;
struct TextureRenderingResource;

class Object {
private:
    glm::vec3 _position;
    glm::vec3 _rotation;
    MeshRenderingResource *_mesh;
    TextureRenderingResource *_texture;

public:
    Object(const glm::vec3 &position, const glm::vec3 &rotation,
           MeshRenderingResource *mesh, TextureRenderingResource *texture);

    glm::mat4 getModelMatrix();

    glm::vec3 &position() { return this->_position; }
    glm::vec3 &rotation() { return this->_rotation; }
    MeshRenderingResource *mesh() { return this->_mesh; }
    TextureRenderingResource *texture() { return this->_texture; }
};

#endif // OBJECT_HPP
