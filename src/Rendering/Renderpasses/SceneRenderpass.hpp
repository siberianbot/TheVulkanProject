#ifndef RENDERING_RENDERPASSES_SCENERENDERPASS_HPP
#define RENDERING_RENDERPASSES_SCENERENDERPASS_HPP

#include <map>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "RenderpassBase.hpp"
#include "src/Rendering/Common.hpp"

class Engine;
class Object;
class Swapchain;
class RenderingObjectsFactory;
class DescriptorSetObject;
class BufferObject;
class ImageObject;
class ImageViewObject;

class SceneRenderpass : public RenderpassBase {
private:
    struct LightData {
        alignas(16) glm::mat4 projection;
        alignas(16) glm::vec3 position;
        alignas(16) glm::vec3 color;
        alignas(4) float radius;
    };

    struct SceneData {
        alignas(16) glm::vec3 cameraPosition;
        alignas(4) int numLights;
        alignas(16) LightData lights[MAX_NUM_LIGHTS];
    };

    struct MeshConstants {
        glm::mat4 matrix;
        glm::mat4 model;
        glm::mat4 modelRotation;
    };

    struct RenderData {
        ImageViewObject *textureView;
        DescriptorSetObject *descriptorSet;
    };

    RenderingObjectsFactory *_renderingObjectsFactory;
    Engine *_engine;
    Swapchain *_swapchain;

    VkSampler _textureSampler;
    std::map<ImageObject *, RenderData> _renderData;
    ImageViewObject *_skyboxTextureView;
    DescriptorSetObject *_skyboxDescriptorSet;

    ImageObject *_skyboxImage;
    ImageViewObject *_skyboxImageView;

    ImageObject *_albedoImage;
    ImageViewObject *_albedoImageView;

    ImageObject *_positionImage;
    ImageViewObject *_positionImageView;

    ImageObject *_normalImage;
    ImageViewObject *_normalImageView;

    ImageObject *_specularImage;
    ImageViewObject *_specularImageView;

    ImageObject *_depthImage;
    ImageViewObject *_depthImageView;

    ImageObject *_compositionImage;
    ImageViewObject *_compositionImageView;

    ImageObject *_resultImage;
    ImageViewObject *_resultImageView;

    VkDescriptorPool _descriptorPool;

    VkDescriptorSetLayout _objectDescriptorSetLayout;
    VkPipelineLayout _skyboxPipelineLayout;
    VkPipeline _skyboxPipeline;
    VkPipelineLayout _scenePipelineLayout;
    VkPipeline _scenePipeline;

    VkDescriptorSetLayout _compositionGBufferDescriptorSetLayout;
    DescriptorSetObject *_compositionGBufferDescriptorSet;
    VkDescriptorSetLayout _compositionSceneDataDescriptorSetLayout;
    DescriptorSetObject *_compositionSceneDataDescriptorSet;
    BufferObject *_compositionSceneDataBuffer;
    SceneData *_compositionSceneData;
    VkPipelineLayout _compositionPipelineLayout;
    VkPipeline _compositionPipeline;

    std::vector<RenderpassBase *> _shadowRenderpasses;

    RenderData getRenderData(Object *object);

    DescriptorSetObject *createTextureDescriptorSetFor(ImageViewObject *imageViewObject);

    void initSkyboxPipeline();
    void destroySkyboxPipeline();

    void initScenePipeline();
    void destroyScenePipeline();

    void initCompositionPipeline();
    void destroyCompositionPipeline();

public:
    SceneRenderpass(RenderingDevice *renderingDevice, Swapchain *swapchain,
                    RenderingObjectsFactory *renderingObjectsFactory, Engine *engine);
    ~SceneRenderpass() override = default;

    void recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                        uint32_t frameIdx, uint32_t imageIdx) override;

    void initRenderpass() override;
    void destroyRenderpass() override;

    void createFramebuffers() override;
    void destroyFramebuffers() override;

    ImageViewObject *getResultImageView(uint32_t imageIdx) override { return this->_resultImageView; }

    // TODO: very stupid way
    void addShadowRenderpass(RenderpassBase *renderpass);
};

#endif // RENDERING_RENDERPASSES_SCENERENDERPASS_HPP
