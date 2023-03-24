#ifndef RENDERING_RENDERPASSES_SHADOWRENDERPASS_HPP
#define RENDERING_RENDERPASSES_SHADOWRENDERPASS_HPP

#include <memory>
#include <vector>

#include "src/Rendering/Renderpasses/RenderpassBase.hpp"
#include "src/Rendering/Types/ModelData.hpp"
#include "src/Rendering/Types/ShadowData.hpp"

class ResourceManager;
class PhysicalDevice;

class ShadowRenderpass : public RenderpassBase {
private:
    std::shared_ptr<PhysicalDevice> _physicalDevice;
    std::shared_ptr<ResourceManager> _resourceManager;

    VkPipelineLayout _pipelineLayout;
    VkPipeline _pipeline;

    VkFramebuffer createFramebuffer(const std::shared_ptr<ImageViewObject> &imageView, VkExtent2D extent) override;

public:
    ShadowRenderpass(const std::shared_ptr<PhysicalDevice> &physicalDevice,
                     const std::shared_ptr<RenderingDevice> &renderingDevice,
                     const std::shared_ptr<ResourceManager> &resourceManager);
    ~ShadowRenderpass() override = default;

    void record(VkCommandBuffer commandBuffer, const ShadowData &shadow, const std::vector<ModelData> &models);

    void beginRenderpass(VkCommandBuffer commandBuffer) override;

    void initRenderpass() override;
    void destroyRenderpass() override;
};

#endif // RENDERING_RENDERPASSES_SHADOWRENDERPASS_HPP
