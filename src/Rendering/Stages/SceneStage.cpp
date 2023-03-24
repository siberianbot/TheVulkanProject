#include "SceneStage.hpp"

#include "src/Engine/EngineVars.hpp"
#include "src/Objects/LightSource.hpp"
#include "src/Objects/Prop.hpp"
#include "src/Objects/Components/ModelComponent.hpp"
#include "src/Objects/Components/PositionComponent.hpp"
#include "src/Rendering/Constants.hpp"
#include "src/Rendering/PhysicalDevice.hpp"
#include "src/Rendering/RenderingManager.hpp"
#include "src/Rendering/Builders/ImageObjectBuilder.hpp"
#include "src/Rendering/Builders/ImageViewObjectBuilder.hpp"
#include "src/Rendering/Objects/ImageObject.hpp"
#include "src/Rendering/Objects/ImageViewObject.hpp"
#include "src/Rendering/Renderpasses/ShadowRenderpass.hpp"
#include "src/Resources/MeshResource.hpp"
#include "src/Scene/Scene.hpp"
#include "src/Scene/SceneNode.hpp"
#include "src/Scene/SceneManager.hpp"

SceneStage::SceneStage(const std::shared_ptr<EngineVars> &engineVars,
                       const std::shared_ptr<EventQueue> &eventQueue,
                       const std::shared_ptr<RenderingManager> &renderingManager,
                       const std::shared_ptr<ResourceManager> &resourceManager,
                       const std::shared_ptr<SceneManager> &sceneManager)
        : _engineVars(engineVars),
          _eventQueue(eventQueue),
          _renderingManager(renderingManager),
          _resourceManager(resourceManager),
          _sceneManager(sceneManager) {
    //
}

void SceneStage::init() {
    this->_shadowMapSize = this->_engineVars->getOrDefault(RENDERING_SCENE_STAGE_SHADOW_MAP_SIZE, 1024)->intValue;
    this->_shadowMapCount = this->_engineVars->getOrDefault(RENDERING_SCENE_STAGE_SHADOW_MAP_COUNT, 32)->intValue;

    this->_shadowMapImage = ImageObjectBuilder(this->_renderingManager->renderingDevice(),
                                               this->_renderingManager->vulkanObjectsAllocator())
            .withUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
            .withFormat(this->_renderingManager->physicalDevice()->getDepthFormat())
            .withExtent(this->_shadowMapSize, this->_shadowMapSize)
            .withSamples(VK_SAMPLE_COUNT_1_BIT)
            .arrayCompatible(this->_shadowMapCount)
            .build();

    this->_shadowMapImageViews.resize(this->_shadowMapCount);
    for (uint32_t idx = 0; idx < this->_shadowMapCount; idx++) {
        this->_shadowMapImageViews[idx] = ImageViewObjectBuilder(this->_renderingManager->vulkanObjectsAllocator())
                .fromImageObject(this->_shadowMapImage)
                .withLayers(idx, 1)
                .withType(VK_IMAGE_VIEW_TYPE_2D)
                .withAspectFlags(VK_IMAGE_ASPECT_DEPTH_BIT)
                .build();
    }

    this->_shadowRenderpass = std::make_unique<ShadowRenderpass>(this->_renderingManager->physicalDevice(),
                                                                 this->_renderingManager->renderingDevice(),
                                                                 this->_resourceManager);
    this->_shadowRenderpass->initRenderpass();
}

void SceneStage::destroy() {
    this->_shadowRenderpass->destroyFramebuffers();
    this->_shadowRenderpass->destroyRenderpass();

    for (const auto &imageView: this->_shadowMapImageViews) {
        imageView->destroy();
    }

    this->_shadowMapImageViews.clear();

    this->_shadowMapImage->destroy();
}

void SceneStage::record(VkCommandBuffer commandBuffer, uint32_t frameIdx, uint32_t imageIdx) {
    std::vector<ShadowData> shadows;
    std::vector<ModelData> models;

    SceneIterator iterator = this->_sceneManager->currentScene()->iterate();

    do {
        std::shared_ptr<SceneNode> node = iterator.current();

        if (node->object() == nullptr) {
            continue;
        }

        std::shared_ptr<LightSource> lightSource = std::dynamic_pointer_cast<LightSource>(node->object());
        std::shared_ptr<Prop> prop = std::dynamic_pointer_cast<Prop>(node->object());

        if (lightSource != nullptr) {
            if (!lightSource->enabled()) {
                continue;
            }

            glm::mat4 projection = lightSource->projection();
            projection[1][1] = -1;

            if (lightSource->type() == POINT_LIGHT_SOURCE) {
                for (glm::vec3 dir: POINT_LIGHT_SOURCE_DIRECTIONS) {
                    shadows.push_back({projection * lightSource->view(dir)});
                }
            } else {
                shadows.push_back({projection * lightSource->view()});
            }
        }

        if (prop != nullptr) {
            auto mesh = prop->model()->mesh().lock();

            ModelData modelData = {
                    .model = prop->position()->model(),
                    .modelRotation = prop->position()->rotationMat4(),
                    .vertices = mesh->vertexBuffer(),
                    .indices = mesh->indexBuffer(),
                    .count = mesh->count()
            };

            models.push_back(modelData);
        }
    } while (iterator.moveNext());

    uint32_t targetCount = std::min(this->_shadowMapCount, (uint32_t) shadows.size());
    VkRect2D renderArea = {
            .offset = {0, 0},
            .extent = {this->_shadowMapSize, this->_shadowMapSize}
    };

    for (uint32_t idx = 0; idx < targetCount; idx++) {
        this->_shadowRenderpass->setTargetImageView(this->_shadowMapImageViews[idx], renderArea);

        this->_shadowRenderpass->beginRenderpass(commandBuffer);
        this->_shadowRenderpass->record(commandBuffer, shadows[idx], models);
        this->_shadowRenderpass->endRenderpass(commandBuffer);
    }
}
