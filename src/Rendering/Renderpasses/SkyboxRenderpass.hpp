#ifndef RENDERING_RENDERPASSES_SKYBOXRENDERPASS_HPP
#define RENDERING_RENDERPASSES_SKYBOXRENDERPASS_HPP

#include "RenderpassBase.hpp"

#include "src/Rendering/RenderingResources.hpp"

class Engine;
class RenderingObjectsFactory;
class DescriptorSetObject;
class ImageObject;
class ImageViewObject;
class RenderingLayoutObject;
class RenderingResourcesManager;

class SkyboxRenderpass : public RenderpassBase {
private:
    Engine *_engine;
    RenderingObjectsFactory *_renderingObjectsFactory;
    ImageViewObject *_textureView;
    VkSampler _textureSampler;
    RenderingLayoutObject *_renderingLayoutObject;
    DescriptorSetObject *_descriptorSet;

    ImageObject *_colorImage = nullptr;
    ImageViewObject *_colorImageView = nullptr;

    VkPipeline _pipeline = VK_NULL_HANDLE;

public:
    SkyboxRenderpass(RenderingDevice *renderingDevice, Swapchain *swapchain,
                     RenderingObjectsFactory *renderingObjectsFactory, Engine *engine);

    ~SkyboxRenderpass() override;

    void recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                        uint32_t frameIdx, uint32_t imageIdx) override;

    void initRenderpass() override;
    void destroyRenderpass() override;

    void createFramebuffers() override;
    void destroyFramebuffers() override;
};

#endif // RENDERING_RENDERPASSES_SKYBOXRENDERPASS_HPP
