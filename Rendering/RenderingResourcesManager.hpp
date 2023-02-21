#ifndef RENDERING_RENDERINGRESOURCESMANAGER_HPP
#define RENDERING_RENDERINGRESOURCESMANAGER_HPP

#include <cstdint>
#include <vector>

#include <vulkan/vulkan.hpp>

struct Vertex;
class Mesh;
class Texture;
class RenderingObjectsFactory;
class CommandExecutor;
class BufferObject;
class ImageObject;

struct MeshResource {
    BufferObject *vertices;
    BufferObject *indices;
    uint32_t indicesCount;
};

struct TextureResource {
    ImageObject *texture;
};

class RenderingResourcesManager {
private:
    RenderingObjectsFactory *_renderingObjectsFactory;
    CommandExecutor *_commandExecutor;

    BufferObject *loadBuffer(uint64_t size, const void* data, VkBufferUsageFlags usage);

public:
    RenderingResourcesManager(RenderingObjectsFactory *renderingObjectsFactory, CommandExecutor *commandExecutor);

    MeshResource loadMesh(Mesh *mesh);
    MeshResource loadMesh(uint32_t count, const Vertex* data);
    TextureResource loadTexture(Texture *texture);

    void freeMesh(const MeshResource &meshResource);
    void freeTexture(const TextureResource &textureResource);
};


#endif // RENDERING_RENDERINGRESOURCESMANAGER_HPP
