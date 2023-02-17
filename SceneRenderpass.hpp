#ifndef SCENERENDERPASS_HPP
#define SCENERENDERPASS_HPP

#include "RenderpassBase.hpp"

class SceneRenderpass : public RenderpassBase {
private:
    VkPipelineLayout _pipelineLayout;
    VkPipeline _pipeline = VK_NULL_HANDLE;
    std::vector<BoundMeshInfo *> _meshes;

    VkShaderModule createShaderModule(const std::string &path);
    void createPipeline();

public:
    SceneRenderpass(const DeviceData &deviceData, VkPipelineLayout pipelineLayout);

    ~SceneRenderpass() override;

    void recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                        uint32_t frameIdx, uint32_t imageIdx) override;

    void createRenderpass() override;

    void addMesh(BoundMeshInfo *mesh);
    void removeMesh(BoundMeshInfo *mesh);
};

#endif // SCENERENDERPASS_HPP
