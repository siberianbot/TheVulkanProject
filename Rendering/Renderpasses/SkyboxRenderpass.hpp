#ifndef SKYBOXRENDERPASS_HPP
#define SKYBOXRENDERPASS_HPP

#include "RenderpassBase.hpp"

class SkyboxRenderpass : RenderpassBase {
private:
    TextureData _skybox;
    BufferData _vertexBuffer;
    std::array<VkDescriptorSet, VK_MAX_INFLIGHT_FRAMES> _descriptorSets;
    VkPipeline _pipeline;
    VkPipelineLayout _pipelineLayout;

public:
    explicit SkyboxRenderpass(const DeviceData &deviceData);

    ~SkyboxRenderpass() override = default;

private:
    void recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                        uint32_t frameIdx, uint32_t imageIdx) override;

    void initRenderpass() override;
};

#endif // SKYBOXRENDERPASS_HPP
