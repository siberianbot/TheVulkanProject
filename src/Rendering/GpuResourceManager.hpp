#ifndef RENDERING_GPURESOURCEMANAGER_HPP
#define RENDERING_GPURESOURCEMANAGER_HPP

#include <map>
#include <memory>
#include <optional>

#include "src/Events/EventHandlerIdx.hpp"
#include "src/Rendering/Types/Mesh.hpp"
#include "src/Resources/ResourceId.hpp"

class Log;
class EventQueue;
class ResourceDatabase;
class ResourceLoader;
class ImageReader;
class MeshReader;

class CommandManager;
class GpuAllocator;
class LogicalDeviceProxy;

class GpuResourceManager {
private:
    std::shared_ptr<Log> _log;
    std::shared_ptr<EventQueue> _eventQueue;
    std::shared_ptr<ResourceDatabase> _resourceDatabase;
    std::shared_ptr<ResourceLoader> _resourceLoader;
    std::shared_ptr<CommandManager> _commandManager;
    std::shared_ptr<GpuAllocator> _allocator;
    std::shared_ptr<LogicalDeviceProxy> _logicalDevice;

    std::shared_ptr<ImageReader> _imageReader;
    std::shared_ptr<MeshReader> _meshReader;

    EventHandlerIdx _handlerIdx;
    std::map<ResourceId, std::shared_ptr<Mesh>> _meshes;

    std::weak_ptr<Mesh> getMesh(const ResourceId &resourceId);

    std::shared_ptr<Mesh> loadMesh(const ResourceId &resourceId);
    void freeMesh(const std::shared_ptr<Mesh> &mesh);

public:
    GpuResourceManager(const std::shared_ptr<Log> &log,
                       const std::shared_ptr<EventQueue> &eventQueue,
                       const std::shared_ptr<ResourceDatabase> resourceDatabase,
                       const std::shared_ptr<ResourceLoader> resourceLoader,
                       const std::shared_ptr<CommandManager> &commandManager,
                       const std::shared_ptr<GpuAllocator> &allocator,
                       const std::shared_ptr<LogicalDeviceProxy> &logicalDevice);

    void init();
    void destroy();

    [[nodiscard]] std::optional<std::weak_ptr<Mesh>> tryGetMesh(const ResourceId &resourceId);

    void free(const ResourceId &resourceId);

    void freeAll();
};

#endif // RENDERING_GPURESOURCEMANAGER_HPP
