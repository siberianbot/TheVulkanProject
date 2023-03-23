#ifndef RENDERING_RENDERER_HPP
#define RENDERING_RENDERER_HPP

#include <array>
#include <memory>
#include <optional>
#include <vector>

#include "src/Rendering/Constants.hpp"

class EventQueue;
class RenderingManager;
class FenceObject;
class SemaphoreObject;
class StageBase;

class Renderer {
private:
    struct SyncObjectsGroup {
        std::shared_ptr<FenceObject> fence;
        std::shared_ptr<SemaphoreObject> imageAvailableSemaphore;
        std::shared_ptr<SemaphoreObject> renderFinishedSemaphore;
    };

    std::shared_ptr<EventQueue> _eventQueue;
    std::shared_ptr<RenderingManager> _renderingManager;

    uint32_t _currentFrameIdx = 0;
    std::array<std::shared_ptr<SyncObjectsGroup>, MAX_INFLIGHT_FRAMES> _syncObjectsGroups;

    std::vector<std::shared_ptr<StageBase>> _stages;

    void handleResize();

    std::optional<uint32_t> acquireNextImageIdx(const std::shared_ptr<SemaphoreObject> &semaphore);

public:
    Renderer(const std::shared_ptr<EventQueue> &eventQueue,
             const std::shared_ptr<RenderingManager> &renderingManager);

    void init();
    void destroy();

    void render();
};

#endif // RENDERING_RENDERER_HPP
