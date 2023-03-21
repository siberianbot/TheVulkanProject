#ifndef OBJECTS_SKYBOX_HPP
#define OBJECTS_SKYBOX_HPP

#include <memory>

class MeshResource;
class CubeImageResource;

class Skybox {
private:
    std::shared_ptr<MeshResource> _mesh;
    std::shared_ptr<CubeImageResource> _texture;

public:
    Skybox(const std::shared_ptr<MeshResource> &mesh, const std::shared_ptr<CubeImageResource> &texture);

    std::shared_ptr<MeshResource> &mesh() { return this->_mesh; }
    std::shared_ptr<CubeImageResource> &texture() { return this->_texture; }
};

#endif // OBJECTS_SKYBOX_HPP
