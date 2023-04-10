#ifndef RENDERING_RENDERINGRESOURCEMANAGER_HPP
#define RENDERING_RENDERINGRESOURCEMANAGER_HPP

#include <map>
#include <memory>
#include <optional>

#include "src/Rendering/Types/MeshRenderingData.hpp"
#include "src/Rendering/Types/TextureRenderingData.hpp"
#include "src/Resources/ResourceId.hpp"

class Log;
class EventQueue;
class RenderingObjectsAllocator;
class VulkanObjectsAllocator;
class BufferObject;
class ImageObject;
class ImageViewObject;
class ResourceDatabase;
class ResourceLoader;
class ImageReader;
class MeshReader;

class RenderingResourceManager {
private:
    std::shared_ptr<Log> _log;
    std::shared_ptr<EventQueue> _eventQueue;
    std::shared_ptr<RenderingObjectsAllocator> _renderingObjectsAllocator;
    std::shared_ptr<VulkanObjectsAllocator> _vulkanObjectsAllocator;
    std::shared_ptr<ResourceDatabase> _resourceDatabase;
    std::shared_ptr<ResourceLoader> _resourceLoader;
    std::shared_ptr<ImageReader> _imageReader;
    std::shared_ptr<MeshReader> _meshReader;

    std::map<ResourceId, MeshRenderingData> _meshes;
    std::map<ResourceId, TextureRenderingData> _textures;

    MeshRenderingData getMesh(const ResourceId &id);
    TextureRenderingData getTexture(const ResourceId &id);

    MeshRenderingData loadMesh(const ResourceId &id);
    TextureRenderingData loadTexture(const ResourceId &id);

    void free(const ResourceId &id);

public:
    RenderingResourceManager(const std::shared_ptr<Log> &log,
                             const std::shared_ptr<EventQueue> &eventQueue,
                             const std::shared_ptr<RenderingObjectsAllocator> &renderingObjectsAllocator,
                             const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator,
                             const std::shared_ptr<ResourceDatabase> &resourceDatabase,
                             const std::shared_ptr<ResourceLoader> &resourceLoader,
                             const std::shared_ptr<ImageReader> &imageReader,
                             const std::shared_ptr<MeshReader> &meshReader);

    std::optional<MeshRenderingData> tryGetMesh(const ResourceId &id);
    std::optional<TextureRenderingData> tryGetTexture(const ResourceId &id);
};

#endif // RENDERING_RENDERINGRESOURCEMANAGER_HPP
