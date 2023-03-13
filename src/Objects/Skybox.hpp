#ifndef OBJECTS_SKYBOX_HPP
#define OBJECTS_SKYBOX_HPP

struct MeshRenderingResource;
struct TextureRenderingResource;

class Skybox {
private:
    MeshRenderingResource *_mesh;
    TextureRenderingResource *_texture;

public:
    Skybox(MeshRenderingResource *mesh, TextureRenderingResource *texture);

    MeshRenderingResource *mesh() { return this->_mesh; }
    TextureRenderingResource *texture() { return this->_texture; }
};

#endif // OBJECTS_SKYBOX_HPP
