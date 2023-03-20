#ifndef RENDERING_RENDERPASSES_SCENERENDERPASS_HPP
#define RENDERING_RENDERPASSES_SCENERENDERPASS_HPP

#include <array>
#include <map>
#include <memory>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "RenderpassBase.hpp"
#include "src/Rendering/Common.hpp"

class Engine;
class Object;
class Swapchain;
struct RenderingData;
class DescriptorSetObject;
class BufferObject;
class ImageObject;
class ImageViewObject;
class PhysicalDevice;
class VulkanObjectsAllocator;

class SceneRenderpass : public RenderpassBase {
private:
    struct LightData {
        alignas(16) glm::mat4 projection;
        alignas(16) glm::vec3 position;
        alignas(16) glm::vec3 color;
        alignas(4) float radius;
        alignas(4) int kind;
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

    std::shared_ptr<PhysicalDevice> _physicalDevice;
    std::shared_ptr<VulkanObjectsAllocator> _vulkanObjectsAllocator;
    Engine *_engine;
    Swapchain *_swapchain;

    VkSampler _textureSampler;
    std::map<std::shared_ptr<ImageObject>, std::shared_ptr<ImageViewObject>> _imageViews;
    std::array<std::shared_ptr<DescriptorSetObject>, MAX_INFLIGHT_FRAMES> _skyboxDescriptorSets;
    std::shared_ptr<ImageViewObject> _skyboxTextureView = nullptr;

    std::shared_ptr<ImageObject> _skyboxImage;
    std::shared_ptr<ImageViewObject> _skyboxImageView;

    std::shared_ptr<ImageObject> _albedoImage;
    std::shared_ptr<ImageViewObject> _albedoImageView;

    std::shared_ptr<ImageObject> _positionImage;
    std::shared_ptr<ImageViewObject> _positionImageView;

    std::shared_ptr<ImageObject> _normalImage;
    std::shared_ptr<ImageViewObject> _normalImageView;

    std::shared_ptr<ImageObject> _specularImage;
    std::shared_ptr<ImageViewObject> _specularImageView;

    std::shared_ptr<ImageObject> _depthImage;
    std::shared_ptr<ImageViewObject> _depthImageView;

    std::shared_ptr<ImageObject> _compositionImage;
    std::shared_ptr<ImageViewObject> _compositionImageView;

    std::shared_ptr<ImageObject> _resultImage;
    std::shared_ptr<ImageViewObject> _resultImageView;

    VkDescriptorPool _descriptorPool;

    VkDescriptorSetLayout _objectDescriptorSetLayout;
    VkPipelineLayout _skyboxPipelineLayout;
    VkPipeline _skyboxPipeline;
    VkPipelineLayout _scenePipelineLayout;
    VkPipeline _scenePipeline;

    VkDescriptorSetLayout _compositionGBufferDescriptorSetLayout;
    std::array<std::shared_ptr<DescriptorSetObject>, MAX_INFLIGHT_FRAMES> _compositionGBufferDescriptorSets;
    VkDescriptorSetLayout _compositionSceneDataDescriptorSetLayout;
    std::array<std::shared_ptr<DescriptorSetObject>, MAX_INFLIGHT_FRAMES> _compositionSceneDataDescriptorSets;
    std::shared_ptr<BufferObject> _compositionSceneDataBuffer;
    SceneData *_compositionSceneData;
    VkPipelineLayout _compositionPipelineLayout;
    VkPipeline _compositionPipeline;

    std::vector<RenderpassBase *> _shadowRenderpasses;

    std::shared_ptr<RenderingData> getRenderData(Object *object);
    std::shared_ptr<ImageViewObject> getImageView(const std::shared_ptr<ImageObject> &image);

    void updateDescriptorSetWithImage(
            const std::array<std::shared_ptr<DescriptorSetObject>, MAX_INFLIGHT_FRAMES> &descriptorSets,
            const std::shared_ptr<ImageViewObject> &imageViewObject,
            uint32_t binding);

    void initSkyboxPipeline();
    void destroySkyboxPipeline();

    void initScenePipeline();
    void destroyScenePipeline();

    void initCompositionPipeline();
    void destroyCompositionPipeline();

public:
    SceneRenderpass(const std::shared_ptr<RenderingDevice> &renderingDevice,
                    const std::shared_ptr<PhysicalDevice> &physicalDevice,
                    const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator,
                    Swapchain *swapchain, Engine *engine);
    ~SceneRenderpass() override = default;

    void recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                        uint32_t frameIdx, uint32_t imageIdx) override;

    void initRenderpass() override;
    void destroyRenderpass() override;

    void createFramebuffers() override;
    void destroyFramebuffers() override;

    std::shared_ptr<ImageViewObject> getResultImageView(uint32_t imageIdx) override { return this->_resultImageView; }

    // TODO: very stupid way
    void addShadowRenderpass(RenderpassBase *renderpass);
};

#endif // RENDERING_RENDERPASSES_SCENERENDERPASS_HPP
