#ifndef RENDERING_RENDERINGRESOURCESMANAGER_HPP
#define RENDERING_RENDERINGRESOURCESMANAGER_HPP

#include <cstdint>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "src/Rendering/RenderingResources.hpp"

struct Vertex;
class Mesh;
class Texture;
class RenderingObjectsFactory;
class CommandExecutor;

class RenderingResourcesManager {
private:
    RenderingObjectsFactory *_renderingObjectsFactory;
    CommandExecutor *_commandExecutor;

    BufferObject *loadBuffer(uint64_t size, const void* data, VkBufferUsageFlags usage);

public:
    RenderingResourcesManager(RenderingObjectsFactory *renderingObjectsFactory, CommandExecutor *commandExecutor);

    MeshRenderingResource loadMesh(Mesh *mesh);
    MeshRenderingResource loadMesh(uint32_t count, const Vertex* data);
    TextureRenderingResource loadTexture(Texture *texture);

    void freeMesh(const MeshRenderingResource &meshResource);
    void freeTexture(const TextureRenderingResource &textureResource);
};


#endif // RENDERING_RENDERINGRESOURCESMANAGER_HPP