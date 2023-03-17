#ifndef OBJECTS_OBJECT_HPP
#define OBJECTS_OBJECT_HPP

#include <map>
#include <memory>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "src/Objects/Data/DataType.hpp"

struct IData;
class MeshResource;
class ImageResource;

class Object {
private:
    glm::vec3 _position = glm::vec3(0);
    glm::vec3 _rotation = glm::vec3(0);
    glm::vec3 _scale = glm::vec3(1);
    MeshResource *_mesh = nullptr;
    ImageResource *_albedoTexture = nullptr;
    ImageResource *_specTexture = nullptr;
    // TODO: weak_ptr
    std::map<DataType, std::shared_ptr<IData>> _data;

public:
    glm::mat4 getModelMatrix(bool rotationOnly);

    glm::vec3 &position() { return this->_position; }
    glm::vec3 &rotation() { return this->_rotation; }
    glm::vec3 &scale() { return this->_scale; }
    MeshResource *&mesh() { return this->_mesh; }
    ImageResource *&albedoTexture() { return this->_albedoTexture; }
    ImageResource *&specTexture() { return this->_specTexture; }
    std::map<DataType, std::shared_ptr<IData>> &data() { return this->_data; }
};

#endif // OBJECTS_OBJECT_HPP
