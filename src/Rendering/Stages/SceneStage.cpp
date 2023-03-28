#include "SceneStage.hpp"

#include "src/Engine/VarCollection.hpp"
#include "src/Engine/Vars.hpp"
#include "src/Events/EventQueue.hpp"
#include "src/Objects/Camera.hpp"
#include "src/Objects/LightSource.hpp"
#include "src/Objects/Prop.hpp"
#include "src/Objects/World.hpp"
#include "src/Objects/Components/ModelComponent.hpp"
#include "src/Objects/Components/PositionComponent.hpp"
#include "src/Objects/Components/SkyboxComponent.hpp"
#include "src/Rendering/PhysicalDevice.hpp"
#include "src/Rendering/RenderingManager.hpp"
#include "src/Rendering/Swapchain.hpp"
#include "src/Rendering/Builders/ImageObjectBuilder.hpp"
#include "src/Rendering/Builders/ImageViewObjectBuilder.hpp"
#include "src/Rendering/Objects/DescriptorSetObject.hpp"
#include "src/Rendering/Objects/ImageObject.hpp"
#include "src/Rendering/Objects/ImageViewObject.hpp"
#include "src/Rendering/Renderpasses/ShadowRenderpass.hpp"
#include "src/Rendering/Renderpasses/SceneRenderpass.hpp"
#include "src/Resources/MeshResource.hpp"
#include "src/Resources/ImageResource.hpp"
#include "src/Resources/ResourceManager.hpp"
#include "src/Scene/Scene.hpp"
#include "src/Scene/SceneNode.hpp"
#include "src/Scene/SceneManager.hpp"

void SceneStage::initGBuffer() {
    VkExtent2D extent = this->_renderingManager->swapchain()->getSwapchainExtent();
    VkSampleCountFlagBits samples = this->_renderingManager->physicalDevice()->getMsaaSamples();

    this->_albedoGBufferImage = ImageObjectBuilder(this->_renderingManager->renderingDevice(),
                                                   this->_renderingManager->vulkanObjectsAllocator())
            .withFormat(VK_FORMAT_R8G8B8A8_UNORM)
            .withSamples(samples)
            .withExtent(extent.width, extent.height)
            .withUsage(VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
            .build();

    this->_albedoGBufferImageView = ImageViewObjectBuilder(this->_renderingManager->vulkanObjectsAllocator())
            .fromImageObject(this->_albedoGBufferImage)
            .withAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT)
            .build();

    this->_positionGBufferImage = ImageObjectBuilder(this->_renderingManager->renderingDevice(),
                                                     this->_renderingManager->vulkanObjectsAllocator())
            .withFormat(VK_FORMAT_R16G16B16A16_SFLOAT)
            .withSamples(samples)
            .withExtent(extent.width, extent.height)
            .withUsage(VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
            .build();

    this->_positionGBufferImageView = ImageViewObjectBuilder(this->_renderingManager->vulkanObjectsAllocator())
            .fromImageObject(this->_positionGBufferImage)
            .withAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT)
            .build();

    this->_normalGBufferImage = ImageObjectBuilder(this->_renderingManager->renderingDevice(),
                                                   this->_renderingManager->vulkanObjectsAllocator())
            .withFormat(VK_FORMAT_R16G16B16A16_SFLOAT)
            .withSamples(samples)
            .withExtent(extent.width, extent.height)
            .withUsage(VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
            .build();

    this->_normalGBufferImageView = ImageViewObjectBuilder(this->_renderingManager->vulkanObjectsAllocator())
            .fromImageObject(this->_normalGBufferImage)
            .withAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT)
            .build();

    this->_specularGBufferImage = ImageObjectBuilder(this->_renderingManager->renderingDevice(),
                                                     this->_renderingManager->vulkanObjectsAllocator())
            .withFormat(VK_FORMAT_R8_UNORM)
            .withSamples(samples)
            .withExtent(extent.width, extent.height)
            .withUsage(VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
            .build();

    this->_specularGBufferImageView = ImageViewObjectBuilder(this->_renderingManager->vulkanObjectsAllocator())
            .fromImageObject(this->_specularGBufferImage)
            .withAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT)
            .build();

    this->_depthGBufferImage = ImageObjectBuilder(this->_renderingManager->renderingDevice(),
                                                  this->_renderingManager->vulkanObjectsAllocator())
            .withFormat(this->_renderingManager->physicalDevice()->getDepthFormat())
            .withSamples(samples)
            .withExtent(extent.width, extent.height)
            .withUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
            .build();

    this->_depthGBufferImageView = ImageViewObjectBuilder(this->_renderingManager->vulkanObjectsAllocator())
            .fromImageObject(this->_depthGBufferImage)
            .withAspectFlags(VK_IMAGE_ASPECT_DEPTH_BIT)
            .build();

    this->_compositionGBufferImage = ImageObjectBuilder(this->_renderingManager->renderingDevice(),
                                                        this->_renderingManager->vulkanObjectsAllocator())
            .withFormat(this->_renderingManager->physicalDevice()->getColorFormat())
            .withSamples(samples)
            .withExtent(extent.width, extent.height)
            .withUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
            .build();

    this->_compositionGBufferImageView = ImageViewObjectBuilder(this->_renderingManager->vulkanObjectsAllocator())
            .fromImageObject(this->_compositionGBufferImage)
            .withAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT)
            .build();

    this->_sceneRenderpass->setGBufferImageViews(this->_albedoGBufferImageView,
                                                 this->_positionGBufferImageView,
                                                 this->_normalGBufferImageView,
                                                 this->_specularGBufferImageView,
                                                 this->_depthGBufferImageView,
                                                 this->_compositionGBufferImageView);
}

void SceneStage::destroyGBuffer() {
    this->_compositionGBufferImageView->destroy();
    this->_compositionGBufferImage->destroy();

    this->_depthGBufferImageView->destroy();
    this->_depthGBufferImage->destroy();

    this->_specularGBufferImageView->destroy();
    this->_specularGBufferImage->destroy();

    this->_normalGBufferImageView->destroy();
    this->_normalGBufferImage->destroy();

    this->_positionGBufferImageView->destroy();
    this->_positionGBufferImage->destroy();

    this->_albedoGBufferImageView->destroy();
    this->_albedoGBufferImage->destroy();
}

SceneStage::SceneStage(const std::shared_ptr<VarCollection> &vars,
                       const std::shared_ptr<EventQueue> &eventQueue,
                       const std::shared_ptr<RenderingManager> &renderingManager,
                       const std::shared_ptr<ResourceManager> &resourceManager,
                       const std::shared_ptr<SceneManager> &sceneManager)
        : _vars(vars),
          _eventQueue(eventQueue),
          _renderingManager(renderingManager),
          _resourceManager(resourceManager),
          _sceneManager(sceneManager) {
    //
}

void SceneStage::init() {
    this->_shadowMapSize = this->_vars->getOrDefault(RENDERING_SCENE_STAGE_SHADOW_MAP_SIZE, 1024);
    this->_shadowMapCount = this->_vars->getOrDefault(RENDERING_SCENE_STAGE_SHADOW_MAP_COUNT, 32);
    this->_lightCount = this->_vars->getOrDefault(RENDERING_SCENE_STAGE_LIGHT_COUNT, 128);

    this->_shadowMapImage = ImageObjectBuilder(this->_renderingManager->renderingDevice(),
                                               this->_renderingManager->vulkanObjectsAllocator())
            .withUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .withFormat(this->_renderingManager->physicalDevice()->getDepthFormat())
            .withExtent(this->_shadowMapSize, this->_shadowMapSize)
            .withSamples(VK_SAMPLE_COUNT_1_BIT)
            .arrayCompatible(this->_shadowMapCount)
            .build();

    this->_shadowMapArrayView = ImageViewObjectBuilder(this->_renderingManager->vulkanObjectsAllocator())
            .fromImageObject(this->_shadowMapImage)
            .withLayers(0, this->_shadowMapCount)
            .withType(VK_IMAGE_VIEW_TYPE_2D_ARRAY)
            .withAspectFlags(VK_IMAGE_ASPECT_DEPTH_BIT)
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

    this->_shadowRenderpass = std::make_unique<ShadowRenderpass>(this->_renderingManager->renderingDevice(),
                                                                 this->_renderingManager->physicalDevice(),
                                                                 this->_renderingManager->renderingLayoutsManager(),
                                                                 this->_renderingManager->vulkanObjectsAllocator(),
                                                                 this->_resourceManager);
    this->_shadowRenderpass->initRenderpass();

    this->_sceneRenderpass = std::make_unique<SceneRenderpass>(this->_renderingManager->renderingDevice(),
                                                               this->_vars,
                                                               this->_renderingManager->physicalDevice(),
                                                               this->_renderingManager->renderingLayoutsManager(),
                                                               this->_renderingManager->vulkanObjectsAllocator(),
                                                               this->_renderingManager->swapchain(),
                                                               this->_resourceManager);
    this->_sceneRenderpass->initRenderpass();

    this->initGBuffer();

    this->_sceneRenderpass->setShadowMapArrayView(this->_shadowMapArrayView);

    this->_eventQueue->addHandler([&](const Event &event) {
        if (event.type != RESIZE_WINDOW_EVENT) {
            return;
        }

        this->destroyGBuffer();
        this->initGBuffer();
    });
}

void SceneStage::destroy() {
    this->destroyGBuffer();

    this->_sceneRenderpass->destroyFramebuffers();
    this->_sceneRenderpass->destroyRenderpass();

    this->_shadowRenderpass->destroyFramebuffers();
    this->_shadowRenderpass->destroyRenderpass();

    for (const auto &imageView: this->_shadowMapImageViews) {
        imageView->destroy();
    }

    this->_shadowMapImageViews.clear();

    this->_shadowMapArrayView->destroy();
    this->_shadowMapImage->destroy();
}

void SceneStage::record(VkCommandBuffer commandBuffer, uint32_t frameIdx, uint32_t imageIdx) {
    std::vector<ShadowData> shadows;
    std::vector<LightData> lights;
    std::vector<ModelData> models;
    CameraData cameraData;
    SkyboxData skybox;
    VkRect2D renderArea;

    VkExtent2D extent = this->_renderingManager->swapchain()->getSwapchainExtent();

    if (!this->_sceneManager->currentCamera().expired()) {
        auto camera = this->_sceneManager->currentCamera().lock();

        glm::mat4 projection = camera->projection((float) extent.width / extent.height);

        projection[1][1] *= -1;

        cameraData = CameraData{
                .matrixAll = projection * camera->view(false),
                .matrixRot = projection * camera->view(true),
                .position = camera->position()->position
        };
    } else {
        cameraData = CameraData{
                .matrixAll = glm::mat4(1),
                .matrixRot = glm::mat4(1),
                .position = glm::vec3(0)
        };
    }

    SceneIterator iterator = this->_sceneManager->currentScene()->iterate();

    do {
        std::shared_ptr<SceneNode> node = iterator.current();

        if (node->object() == nullptr) {
            continue;
        }

        std::shared_ptr<LightSource> lightSource = std::dynamic_pointer_cast<LightSource>(node->object());
        std::shared_ptr<Prop> prop = std::dynamic_pointer_cast<Prop>(node->object());
        std::shared_ptr<World> world = std::dynamic_pointer_cast<World>(node->object());

        if (lightSource != nullptr) {
            if (!lightSource->enabled()) {
                continue;
            }

            glm::mat4 projection = lightSource->projection();
            projection[1][1] = -1;

            if (lightSource->type() == POINT_LIGHT_SOURCE) {
                for (glm::vec3 dir: POINT_LIGHT_SOURCE_DIRECTIONS) {
                    glm::mat4 matrix = projection * lightSource->view(dir);

                    shadows.push_back(ShadowData{
                            .matrix = matrix,
                            .position = lightSource->position()->position,
                            .range = lightSource->range()
                    });

                    lights.push_back(LightData{
                            .matrix = matrix,
                            .position = lightSource->position()->position,
                            .color = lightSource->color(),
                            .range = lightSource->range()
                    });
                }
            } else {
                glm::mat4 matrix = projection * lightSource->view();

                shadows.push_back(ShadowData{
                        .matrix = matrix,
                        .position = lightSource->position()->position,
                        .range = lightSource->range()
                });

                lights.push_back(LightData{
                        .matrix = matrix,
                        .position = lightSource->position()->position,
                        .color = lightSource->color(),
                        .range = lightSource->range()
                });
            }
        }

        if (prop != nullptr) {
            auto mesh = prop->model()->mesh().lock();

            if (prop->model()->mesh().expired()) {
                continue;
            }

            if (prop->model()->isDirty()) {
                if (prop->model()->albedoTextureView() != nullptr) {
                    prop->model()->albedoTextureView()->destroy();
                    prop->model()->albedoTextureView() = nullptr;
                }

                if (prop->model()->specularTextureView() != nullptr) {
                    prop->model()->specularTextureView()->destroy();
                    prop->model()->specularTextureView() = nullptr;
                }

                std::shared_ptr<ImageResource> albedoTexture;
                std::shared_ptr<ImageResource> specularTexture;

                if (!prop->model()->albedoTexture().expired()) {
                    albedoTexture = prop->model()->albedoTexture().lock();
                } else {
                    albedoTexture = this->_resourceManager->loadDefaultImage();
                }

                if (!prop->model()->specularTexture().expired()) {
                    specularTexture = prop->model()->specularTexture().lock();
                } else {
                    specularTexture = this->_resourceManager->loadDefaultImage();
                }

                prop->model()->albedoTextureView() = ImageViewObjectBuilder(
                        this->_renderingManager->vulkanObjectsAllocator())
                        .fromImageObject(albedoTexture->image())
                        .withType(VK_IMAGE_VIEW_TYPE_2D)
                        .withAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT)
                        .build();

                prop->model()->specularTextureView() = ImageViewObjectBuilder(
                        this->_renderingManager->vulkanObjectsAllocator())
                        .fromImageObject(specularTexture->image())
                        .withType(VK_IMAGE_VIEW_TYPE_2D)
                        .withAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT)
                        .build();

                for (uint32_t idx = 0; idx < MAX_INFLIGHT_FRAMES; idx++) {
                    if (prop->model()->albedoTextureView() != nullptr) {
                        prop->model()->descriptorSets()[idx]->updateWithImageView(
                                0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, prop->model()->albedoTextureView(),
                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, this->_sceneRenderpass->textureSampler());
                    }

                    if (prop->model()->specularTextureView() != nullptr) {
                        prop->model()->descriptorSets()[idx]->updateWithImageView(
                                1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, prop->model()->specularTextureView(),
                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, this->_sceneRenderpass->textureSampler());
                    }
                }

                prop->model()->resetDirty();
            }

            ModelData modelData = {
                    .model = prop->position()->model(),
                    .modelRotation = prop->position()->rotationMat4(),
                    .vertices = mesh->vertexBuffer(),
                    .indices = mesh->indexBuffer(),
                    .count = mesh->count(),
                    .descriptorSet = prop->model()->descriptorSets()[frameIdx]
            };

            models.push_back(modelData);
        }

        if (world != nullptr) {
            auto mesh = world->skybox()->mesh().lock();

            skybox = {
                    .vertices = mesh->vertexBuffer(),
                    .indices = mesh->indexBuffer(),
                    .count = mesh->count(),
                    .descriptorSet = world->skybox()->descriptorSets()[frameIdx]
            };
        }
    } while (iterator.moveNext());

    uint32_t targetShadowCount = std::min(this->_shadowMapCount, (uint32_t) shadows.size());
    renderArea = {
            .offset = {0, 0},
            .extent = {this->_shadowMapSize, this->_shadowMapSize}
    };

    for (uint32_t idx = 0; idx < this->_shadowMapCount; idx++) {
        this->_shadowRenderpass->setTargetImageView(this->_shadowMapImageViews[idx], renderArea);

        this->_shadowRenderpass->beginRenderpass(commandBuffer);

        if (idx < targetShadowCount) {
            this->_shadowRenderpass->record(commandBuffer, shadows[idx], models);
        }

        this->_shadowRenderpass->endRenderpass(commandBuffer);
    }

    uint32_t targetLightCount = std::min(this->_lightCount, (uint32_t) lights.size());
    renderArea = {
            .offset = {0, 0},
            .extent = this->_renderingManager->swapchain()->getSwapchainExtent()
    };

    SceneData scene = {
            .ambient = 0.15f,
            .shadowCount = targetShadowCount,
            .lightCount = targetLightCount
    };

    this->_sceneRenderpass->setTargetImageView(this->_renderingManager->swapchain()->getSwapchainImageView(imageIdx),
                                               renderArea);

    this->_sceneRenderpass->beginRenderpass(commandBuffer);
    this->_sceneRenderpass->record(commandBuffer, frameIdx, scene, cameraData, shadows, lights, models);
    this->_sceneRenderpass->endRenderpass(commandBuffer);
}
