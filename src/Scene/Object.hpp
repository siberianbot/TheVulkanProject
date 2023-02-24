#ifndef SCENE_OBJECT_HPP
#define SCENE_OBJECT_HPP

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

struct MeshRenderingResource;
struct TextureRenderingResource;

class Object {
private:
    glm::vec3 _position;
    glm::vec3 _rotation;
    glm::vec3 _scale;
    MeshRenderingResource *_mesh;
    TextureRenderingResource *_texture;

public:
    Object(const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale,
           MeshRenderingResource *mesh, TextureRenderingResource *texture);

    glm::mat4 getModelMatrix();

    glm::vec3 &position() { return this->_position; }
    glm::vec3 &rotation() { return this->_rotation; }
    glm::vec3 &scale() { return this->_scale; }
    MeshRenderingResource *mesh() { return this->_mesh; }
    TextureRenderingResource *texture() { return this->_texture; }
};

#endif // SCENE_OBJECT_HPP
