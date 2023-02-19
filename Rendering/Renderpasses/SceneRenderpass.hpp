#ifndef RENDERING_RENDERPASSES_SCENERENDERPASS_HPP
#define RENDERING_RENDERPASSES_SCENERENDERPASS_HPP

#include "RenderpassBase.hpp"

#include "RendererTypes.hpp"

class SceneRenderpass : public RenderpassBase {
private:
    VkPipelineLayout _pipelineLayout;
    VkPipeline _pipeline = VK_NULL_HANDLE;
    std::vector<BoundMeshInfo *> _meshes;

public:
    SceneRenderpass(RenderingDevice *renderingDevice, Swapchain *swapchain, VkPipelineLayout pipelineLayout);
    ~SceneRenderpass() override = default;

    void recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                        uint32_t frameIdx, uint32_t imageIdx) override;

    void initRenderpass() override;
    void destroyRenderpass() override;

    void addMesh(BoundMeshInfo *mesh);
    void removeMesh(BoundMeshInfo *mesh);
};

#endif // RENDERING_RENDERPASSES_SCENERENDERPASS_HPP
