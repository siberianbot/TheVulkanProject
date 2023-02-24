#include "Object.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

Object::Object(const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale,
               MeshRenderingResource *mesh, TextureRenderingResource *texture)
        : _position(position),
          _rotation(rotation),
          _scale(scale),
          _mesh(mesh),
          _texture(texture) {
    //
}

glm::mat4 Object::getModelMatrix() {
    glm::mat4 t = glm::translate(glm::mat4(1), this->_position);
    glm::mat4 r = toMat4(glm::quat(this->_rotation));
    glm::mat4 s = glm::scale(glm::mat4(1), this->_scale);

    return t * r * s;
}
