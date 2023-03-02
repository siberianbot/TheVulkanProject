#ifndef SCENE_SKYBOX_HPP
#define SCENE_SKYBOX_HPP

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

#endif // SCENE_SKYBOX_HPP
