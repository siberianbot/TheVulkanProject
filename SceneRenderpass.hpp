#ifndef SCENERENDERPASS_HPP
#define SCENERENDERPASS_HPP

#include "RenderpassBase.hpp"

#include "RendererTypes.hpp"

class SceneRenderpass : public RenderpassBase {
private:
    VkPipelineLayout _pipelineLayout;
    VkPipeline _pipeline = VK_NULL_HANDLE;
    std::vector<BoundMeshInfo *> _meshes;

public:
    SceneRenderpass(const RenderingDevice &renderingDevice, VkPipelineLayout pipelineLayout);
    ~SceneRenderpass() override = default;

    void recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                        uint32_t frameIdx, uint32_t imageIdx) override;

    void initRenderpass() override;
    void destroyRenderpass() override;

    void addMesh(BoundMeshInfo *mesh);
    void removeMesh(BoundMeshInfo *mesh);
};

#endif // SCENERENDERPASS_HPP
