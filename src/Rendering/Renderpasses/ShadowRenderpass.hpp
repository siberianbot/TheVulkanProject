#ifndef RENDERING_RENDERPASSES_SHADOWRENDERPASS_HPP
#define RENDERING_RENDERPASSES_SHADOWRENDERPASS_HPP

#include <array>
#include <memory>

#include <glm/mat4x4.hpp>

#include "RenderpassBase.hpp"
#include "src/Rendering/Common.hpp"

class Engine;
class PhysicalDevice;
class VulkanObjectsAllocator;
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
    std::shared_ptr<PhysicalDevice> _physicalDevice;
    std::shared_ptr<VulkanObjectsAllocator> _vulkanObjectsAllocator;

    VkPipelineLayout _pipelineLayout;
    VkPipeline _pipeline;

    std::array<std::shared_ptr<ImageObject>, MAX_NUM_LIGHTS> _depthImages;
    std::array<ImageViewObject *, MAX_NUM_LIGHTS> _depthImageViews;

public:
    ShadowRenderpass(const std::shared_ptr<RenderingDevice> &renderingDevice,
                     const std::shared_ptr<PhysicalDevice> &physicalDevice,
                     const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator,
                     Engine *engine);
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
