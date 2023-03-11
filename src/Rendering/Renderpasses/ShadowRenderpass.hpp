#ifndef RENDERING_RENDERPASSES_SHADOWRENDERPASS_HPP
#define RENDERING_RENDERPASSES_SHADOWRENDERPASS_HPP

#include <array>

#include <glm/mat4x4.hpp>

#include "RenderpassBase.hpp"
#include "src/Rendering/Common.hpp"

class Engine;
class RenderingObjectsFactory;
class ImageObject;

class ShadowRenderpass : public RenderpassBase {
private:
    struct MeshConstants {
        glm::mat4 matrix;
    };

    struct LightData {
        glm::mat4 projection;
        glm::mat4 view;
    };

    Engine *_engine;
    RenderingObjectsFactory *_renderingObjectsFactory;

    VkPipelineLayout _pipelineLayout;
    VkPipeline _pipeline;

    std::array<ImageObject *, MAX_NUM_LIGHTS> _depthImages;
    std::array<ImageViewObject *, MAX_NUM_LIGHTS> _depthImageViews;

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

    ImageViewObject *getResultImageView(uint32_t imageIdx) override { return this->_depthImageViews[imageIdx]; }
};

#endif // RENDERING_RENDERPASSES_SHADOWRENDERPASS_HPP
