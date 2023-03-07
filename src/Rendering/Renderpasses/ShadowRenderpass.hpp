#ifndef RENDERING_RENDERPASSES_SHADOWRENDERPASS_HPP
#define RENDERING_RENDERPASSES_SHADOWRENDERPASS_HPP

#include <glm/mat4x4.hpp>

#include "RenderpassBase.hpp"

class Engine;
class RenderingObjectsFactory;
class ImageObject;

// TODO: should accept multiple light sources
class ShadowRenderpass : public RenderpassBase {
private:
    struct MeshConstants {
        glm::mat4 matrix;
    };

    Engine *_engine;
    RenderingObjectsFactory *_renderingObjectsFactory;

    VkPipelineLayout _pipelineLayout;
    VkPipeline _pipeline;

    ImageObject *_depthImage;
    ImageViewObject *_depthImageView;

public:
    ShadowRenderpass(RenderingDevice *renderingDevice, Engine *engine,
                     RenderingObjectsFactory *renderingObjectsFactory);
    ~ShadowRenderpass() override = default;

    void recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                        uint32_t frameIdx, uint32_t imageIdx) override;

    void initRenderpass() override;
    void destroyRenderpass() override;

    void createFramebuffers() override;
    void destroyFramebuffers() override;

    ImageViewObject *getResultImageView(uint32_t imageIdx) override { return this->_depthImageView; }
};

#endif // RENDERING_RENDERPASSES_SHADOWRENDERPASS_HPP
