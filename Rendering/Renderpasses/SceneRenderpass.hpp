#ifndef RENDERING_RENDERPASSES_SCENERENDERPASS_HPP
#define RENDERING_RENDERPASSES_SCENERENDERPASS_HPP

#include <map>

#include "RenderpassBase.hpp"

class Engine;
class Object;
class RenderingObjectsFactory;
class DescriptorSetObject;
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
    VkSampler _textureSampler;
    std::map<Object *, RenderData> _renderData;

    VkPipeline _pipeline = VK_NULL_HANDLE;

    RenderData getRenderData(Object *object);

public:
    SceneRenderpass(RenderingDevice *renderingDevice, Swapchain *swapchain,
                    RenderingObjectsFactory *renderingObjectsFactory, Engine *engine);
    ~SceneRenderpass() override;

    void recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                        uint32_t frameIdx, uint32_t imageIdx) override;

    void initRenderpass() override;
    void destroyRenderpass() override;
};

#endif // RENDERING_RENDERPASSES_SCENERENDERPASS_HPP
