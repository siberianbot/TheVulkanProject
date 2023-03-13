#ifndef OBJECTS_OBJECT_HPP
#define OBJECTS_OBJECT_HPP

#include <vector>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

struct IData;
struct MeshRenderingResource;
struct TextureRenderingResource;

class Object {
private:
    glm::vec3 _position = glm::vec3(0);
    glm::vec3 _rotation = glm::vec3(0);
    glm::vec3 _scale = glm::vec3(1);
    MeshRenderingResource *_mesh = nullptr;
    TextureRenderingResource *_albedoTexture = nullptr;
    TextureRenderingResource *_specTexture = nullptr;
    std::vector<IData *> _data;

public:
    ~Object();

    glm::mat4 getModelMatrix(bool rotationOnly);

    glm::vec3 &position() { return this->_position; }
    glm::vec3 &rotation() { return this->_rotation; }
    glm::vec3 &scale() { return this->_scale; }
    MeshRenderingResource *&mesh() { return this->_mesh; }
    TextureRenderingResource *&albedoTexture() { return this->_albedoTexture; }
    TextureRenderingResource *&specTexture() { return this->_specTexture; }
    std::vector<IData *> &data() { return this->_data; }
};

#endif // OBJECTS_OBJECT_HPP
