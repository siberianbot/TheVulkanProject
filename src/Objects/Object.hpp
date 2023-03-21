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
    std::shared_ptr<MeshResource> _mesh = nullptr;
    std::shared_ptr<ImageResource> _albedoTexture = nullptr;
    std::shared_ptr<ImageResource> _specTexture = nullptr;
    std::map<DataType, std::shared_ptr<IData>> _data;

public:
    glm::mat4 getModelMatrix(bool rotationOnly);

    glm::vec3 &position() { return this->_position; }
    glm::vec3 &rotation() { return this->_rotation; }
    glm::vec3 &scale() { return this->_scale; }
    std::shared_ptr<MeshResource> &mesh() { return this->_mesh; }
    std::shared_ptr<ImageResource> &albedoTexture() { return this->_albedoTexture; }
    std::shared_ptr<ImageResource> &specTexture() { return this->_specTexture; }
    std::map<DataType, std::shared_ptr<IData>> &data() { return this->_data; }
};

#endif // OBJECTS_OBJECT_HPP
