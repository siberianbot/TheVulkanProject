#ifndef OBJECTS_SKYBOX_HPP
#define OBJECTS_SKYBOX_HPP

class MeshResource;
class CubeImageResource;

class Skybox {
private:
    MeshResource *_mesh;
    CubeImageResource *_texture;

public:
    Skybox(MeshResource *mesh, CubeImageResource *texture);

    MeshResource *mesh() { return this->_mesh; }
    CubeImageResource *texture() { return this->_texture; }
};

#endif // OBJECTS_SKYBOX_HPP
