#ifndef RENDERING_STAGES_SCENESTAGE_HPP
#define RENDERING_STAGES_SCENESTAGE_HPP

#include <memory>
#include <vector>

#include "src/Rendering/Stages/StageBase.hpp"

class EngineVars;
class EventQueue;
class RenderingManager;
class ImageObject;
class ImageViewObject;
class ShadowRenderpass;
class ResourceManager;
class SceneManager;

class SceneStage : public StageBase {
private:
    std::shared_ptr<EngineVars> _engineVars;
    std::shared_ptr<EventQueue> _eventQueue;
    std::shared_ptr<RenderingManager> _renderingManager;
    std::shared_ptr<ResourceManager> _resourceManager;
    std::shared_ptr<SceneManager> _sceneManager;

    uint32_t _shadowMapSize;
    uint32_t _shadowMapCount;
    std::shared_ptr<ImageObject> _shadowMapImage;
    std::vector<std::shared_ptr<ImageViewObject>> _shadowMapImageViews;
    std::unique_ptr<ShadowRenderpass> _shadowRenderpass;

public:
    SceneStage(const std::shared_ptr<EngineVars> &engineVars,
               const std::shared_ptr<EventQueue> &eventQueue,
               const std::shared_ptr<RenderingManager> &renderingManager,
               const std::shared_ptr<ResourceManager> &resourceManager,
               const std::shared_ptr<SceneManager> &sceneManager);
    ~SceneStage() override = default;

    void init() override;
    void destroy() override;

    void record(VkCommandBuffer commandBuffer, uint32_t frameIdx, uint32_t imageIdx) override;
};

#endif // RENDERING_STAGES_SCENESTAGE_HPP
