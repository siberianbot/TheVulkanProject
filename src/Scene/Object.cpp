#include "Object.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "src/Scene/Data/IData.hpp"

Object::~Object() {
    for (IData *data: this->_data) {
        delete data;
    }
}

glm::mat4 Object::getModelMatrix(bool rotationOnly) {
    glm::mat4 r = toMat4(glm::quat(this->_rotation));

    if (rotationOnly) {
        return r;
    }

    glm::mat4 t = glm::translate(glm::mat4(1), this->_position);
    glm::mat4 s = glm::scale(glm::mat4(1), this->_scale);

    return t * r * s;
}
