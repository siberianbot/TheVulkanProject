#ifndef RENDERING_RENDERPASSES_SKYBOXRENDERPASS_HPP
#define RENDERING_RENDERPASSES_SKYBOXRENDERPASS_HPP

#include "RenderpassBase.hpp"

#include "src/Rendering/RenderingResources.hpp"

class Engine;
class RenderingObjectsFactory;
class DescriptorSetObject;
class ImageViewObject;
class RenderingLayoutObject;
class RenderingResourcesManager;

class SkyboxRenderpass : public RenderpassBase {
private:
    Engine *_engine;
    RenderingResourcesManager *_renderingResourcesManager;
    MeshRenderingResource _meshResource;
    TextureRenderingResource _textureResource;
    ImageViewObject *_textureView;
    VkSampler _textureSampler;
    RenderingLayoutObject *_renderingLayoutObject;
    DescriptorSetObject *_descriptorSet;

    VkPipeline _pipeline = VK_NULL_HANDLE;

public:
    SkyboxRenderpass(RenderingDevice *renderingDevice, Swapchain *swapchain,
                     RenderingObjectsFactory *renderingObjectsFactory,
                     RenderingResourcesManager *renderingResourcesManager, Engine *engine);

    ~SkyboxRenderpass() override;

    void recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                        uint32_t frameIdx, uint32_t imageIdx) override;

    void initRenderpass() override;
    void destroyRenderpass() override;

    void createFramebuffers() override;
};

#endif // RENDERING_RENDERPASSES_SKYBOXRENDERPASS_HPP
