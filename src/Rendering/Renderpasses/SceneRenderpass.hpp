#ifndef RENDERING_RENDERPASSES_SCENERENDERPASS_HPP
#define RENDERING_RENDERPASSES_SCENERENDERPASS_HPP

#include <memory>

#include "src/Rendering/Constants.hpp"
#include "src/Rendering/Renderpasses/RenderpassBase.hpp"
#include "src/Rendering/Types/CameraData.hpp"
#include "src/Rendering/Types/LightData.hpp"
#include "src/Rendering/Types/ModelData.hpp"
#include "src/Rendering/Types/SceneData.hpp"
#include "src/Rendering/Types/ShadowData.hpp"
#include "src/Rendering/Types/SkyboxData.hpp"

class EngineVars;
class PhysicalDevice;
class RenderingLayoutsManager;
class VulkanObjectsAllocator;
class Swapchain;
class BufferObject;
class DescriptorSetObject;
class ImageViewObject;
class ResourceManager;

class SceneRenderpass : public RenderpassBase {
private:
    struct SceneConstants {
        uint32_t shadowCount;
        uint32_t lightCount;
    };

    SceneConstants _sceneConstants;

    std::shared_ptr<EngineVars> _engineVars;
    std::shared_ptr<PhysicalDevice> _physicalDevice;
    std::shared_ptr<RenderingLayoutsManager> _renderingLayoutsManager;
    std::shared_ptr<VulkanObjectsAllocator> _vulkanObjectsAllocator;
    std::shared_ptr<Swapchain> _swapchain;
    std::shared_ptr<ResourceManager> _resourceManager;

    std::weak_ptr<ImageViewObject> _albedoImageView;
    std::weak_ptr<ImageViewObject> _positionImageView;
    std::weak_ptr<ImageViewObject> _normalImageView;
    std::weak_ptr<ImageViewObject> _specularImageView;
    std::weak_ptr<ImageViewObject> _depthImageView;
    std::weak_ptr<ImageViewObject> _compositionImageView;

    std::array<std::shared_ptr<DescriptorSetObject>, MAX_INFLIGHT_FRAMES> _compositionDescriptorSets;
    std::shared_ptr<BufferObject> _shadowDataBuffer;
    std::shared_ptr<BufferObject> _lightDataBuffer;
    std::shared_ptr<BufferObject> _cameraDataBuffer;
    std::shared_ptr<BufferObject> _sceneDataBuffer;
    ShadowData *_shadowData;
    LightData *_lightData;
    CameraData *_cameraData;
    SceneData *_sceneData;

    VkSampler _textureSampler;
    VkSampler _shadowMapSampler;

    VkPipeline _modelPipeline;
    VkPipeline _compositionPipeline;

    VkFramebuffer createFramebuffer(const std::shared_ptr<ImageViewObject> &imageView, VkExtent2D extent) override;

public:
    SceneRenderpass(const std::shared_ptr<RenderingDevice> &renderingDevice,
                    const std::shared_ptr<EngineVars> &engineVars,
                    const std::shared_ptr<PhysicalDevice> &physicalDevice,
                    const std::shared_ptr<RenderingLayoutsManager> &renderingLayoutsManager,
                    const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator,
                    const std::shared_ptr<Swapchain> &swapchain,
                    const std::shared_ptr<ResourceManager> &resourceManager);
    ~SceneRenderpass() override = default;

    void record(VkCommandBuffer commandBuffer, uint32_t frameIdx,
                const SceneData &sceneData,
                const CameraData &cameraData,
                const std::vector<ShadowData> &shadowData,
                const std::vector<LightData> &lightData,
                const std::vector<ModelData> &models);

    void beginRenderpass(VkCommandBuffer commandBuffer) override;

    void initRenderpass() override;
    void destroyRenderpass() override;

    void setGBufferImageViews(const std::shared_ptr<ImageViewObject> &albedoImageView,
                              const std::shared_ptr<ImageViewObject> &positionImageView,
                              const std::shared_ptr<ImageViewObject> &normalImageView,
                              const std::shared_ptr<ImageViewObject> &specularImageView,
                              const std::shared_ptr<ImageViewObject> &depthImageView,
                              const std::shared_ptr<ImageViewObject> &compositionImageView);

    void setShadowMapArrayView(const std::shared_ptr<ImageViewObject> &shadowMapArrayView);

    [[nodiscard]] VkSampler textureSampler() const { return this->_textureSampler; }
};

#endif // RENDERING_RENDERPASSES_SCENERENDERPASS_HPP
