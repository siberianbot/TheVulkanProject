#ifndef RENDERING_RENDERPASSES_SCENERENDERPASS_HPP
#define RENDERING_RENDERPASSES_SCENERENDERPASS_HPP

#include <map>

#include "RenderpassBase.hpp"

class Engine;
class Object;
class Swapchain;
class RenderingObjectsFactory;
class DescriptorSetObject;
class ImageObject;
class ImageViewObject;
class RenderingLayoutObject;

class SceneRenderpass : public RenderpassBase {
private:
    struct RenderData {
        ImageViewObject *textureView;
        DescriptorSetObject *descriptorSet;
    };

    RenderingObjectsFactory *_renderingObjectsFactory;
    Engine *_engine;
    RenderingLayoutObject *_renderingLayoutObject;
    Swapchain *_swapchain;
    VkSampler _textureSampler;
    std::map<Object *, RenderData> _renderData;
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

    VkPipeline _skyboxPipeline = VK_NULL_HANDLE;
    VkPipeline _scenePipeline = VK_NULL_HANDLE;
    VkPipeline _compositionPipeline;

    RenderData getRenderData(Object *object);

public:
    SceneRenderpass(RenderingDevice *renderingDevice, Swapchain *swapchain,
                    RenderingObjectsFactory *renderingObjectsFactory, Engine *engine);
    ~SceneRenderpass() override;

    void recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                        uint32_t frameIdx, uint32_t imageIdx) override;

    void initRenderpass() override;
    void destroyRenderpass() override;

    void createFramebuffers() override;
    void destroyFramebuffers() override;

    ImageViewObject *getResultImageView(uint32_t imageIdx) override { return this->_resultImageView; }
};

#endif // RENDERING_RENDERPASSES_SCENERENDERPASS_HPP
