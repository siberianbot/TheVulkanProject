#include "Object.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

Object::Object(const glm::vec3 &position, const glm::vec3 &rotation,
               MeshRenderingResource *mesh, TextureRenderingResource *texture)
        : _position(position),
          _rotation(rotation),
          _mesh(mesh),
          _texture(texture) {
    //
}

glm::mat4 Object::getModelMatrix() {
    glm::mat4 t = glm::translate(glm::mat4(1), this->_position);
    glm::quat r = glm::quat(this->_rotation);

    return t * toMat4(r);
}
