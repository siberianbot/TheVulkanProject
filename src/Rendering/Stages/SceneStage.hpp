#ifndef RENDERING_STAGES_SCENESTAGE_HPP
#define RENDERING_STAGES_SCENESTAGE_HPP

#include <memory>
#include <vector>

#include "src/Rendering/Stages/StageBase.hpp"

class VarCollection;
class EventQueue;
class RenderingManager;
class ImageObject;
class ImageViewObject;
class ShadowRenderpass;
class SceneRenderpass;
class ResourceDatabase;
class ResourceLoader;
class SceneManager;

class SceneStage : public StageBase {
private:
    std::shared_ptr<VarCollection> _vars;
    std::shared_ptr<EventQueue> _eventQueue;
    std::shared_ptr<RenderingManager> _renderingManager;
    std::shared_ptr<ResourceDatabase> _resourceDatabase;
    std::shared_ptr<ResourceLoader> _resourceLoader;
    std::shared_ptr<SceneManager> _sceneManager;

    uint32_t _shadowMapSize;
    uint32_t _shadowMapCount;
    std::shared_ptr<ImageObject> _shadowMapImage;
    std::shared_ptr<ImageViewObject> _shadowMapArrayView;
    std::vector<std::shared_ptr<ImageViewObject>> _shadowMapImageViews;
    std::unique_ptr<ShadowRenderpass> _shadowRenderpass;

    uint32_t _lightCount;
    std::shared_ptr<ImageObject> _albedoGBufferImage;
    std::shared_ptr<ImageObject> _positionGBufferImage;
    std::shared_ptr<ImageObject> _normalGBufferImage;
    std::shared_ptr<ImageObject> _specularGBufferImage;
    std::shared_ptr<ImageObject> _depthGBufferImage;
    std::shared_ptr<ImageObject> _compositionGBufferImage;
    std::shared_ptr<ImageViewObject> _albedoGBufferImageView;
    std::shared_ptr<ImageViewObject> _positionGBufferImageView;
    std::shared_ptr<ImageViewObject> _normalGBufferImageView;
    std::shared_ptr<ImageViewObject> _specularGBufferImageView;
    std::shared_ptr<ImageViewObject> _depthGBufferImageView;
    std::shared_ptr<ImageViewObject> _compositionGBufferImageView;
    std::unique_ptr<SceneRenderpass> _sceneRenderpass;

    void initGBuffer();
    void destroyGBuffer();

public:
    SceneStage(const std::shared_ptr<VarCollection> &vars,
               const std::shared_ptr<EventQueue> &eventQueue,
               const std::shared_ptr<RenderingManager> &renderingManager,
               const std::shared_ptr<ResourceDatabase> &resourceDatabase,
               const std::shared_ptr<ResourceLoader> &resourceLoader,
               const std::shared_ptr<SceneManager> &sceneManager);
    ~SceneStage() override = default;

    void init() override;
    void destroy() override;

    void record(VkCommandBuffer commandBuffer, uint32_t frameIdx, uint32_t imageIdx) override;
};

#endif // RENDERING_STAGES_SCENESTAGE_HPP
