#ifndef RENDERING_RENDERER_HPP
#define RENDERING_RENDERER_HPP

#include <array>
#include <memory>
#include <optional>
#include <vector>

#include "src/Rendering/Constants.hpp"

class VarCollection;
class EventQueue;
class RenderingManager;
class FenceObject;
class SemaphoreObject;
class StageBase;
class ResourceManager;
class SceneManager;

class Renderer {
private:
    struct SyncObjectsGroup {
        std::shared_ptr<FenceObject> fence;
        std::shared_ptr<SemaphoreObject> imageAvailableSemaphore;
        std::shared_ptr<SemaphoreObject> renderFinishedSemaphore;
    };

    std::shared_ptr<VarCollection> _vars;
    std::shared_ptr<EventQueue> _eventQueue;
    std::shared_ptr<RenderingManager> _renderingManager;
    std::shared_ptr<ResourceManager> _resourceManager;
    std::shared_ptr<SceneManager> _sceneManager;

    uint32_t _currentFrameIdx = 0;
    std::array<std::shared_ptr<SyncObjectsGroup>, MAX_INFLIGHT_FRAMES> _syncObjectsGroups;

    std::vector<std::shared_ptr<StageBase>> _stages;

    void handleResize();

    std::optional<uint32_t> acquireNextImageIdx(const std::shared_ptr<SemaphoreObject> &semaphore);

public:
    Renderer(const std::shared_ptr<VarCollection> &vars,
             const std::shared_ptr<EventQueue> &eventQueue,
             const std::shared_ptr<RenderingManager> &renderingManager,
             const std::shared_ptr<ResourceManager> &resourceManager,
             const std::shared_ptr<SceneManager> &sceneManager);

    void init();
    void destroy();

    void render();
};

#endif // RENDERING_RENDERER_HPP
