#ifndef RENDERING_RENDERINGRESOURCES_HPP
#define RENDERING_RENDERINGRESOURCES_HPP

class BufferObject;
class ImageObject;

struct MeshRenderingResource {
    BufferObject *vertices;
    BufferObject *indices;
    uint32_t count;
};

struct TextureRenderingResource {
    ImageObject *texture;
};

#endif // RENDERING_RENDERINGRESOURCES_HPP
