#ifndef SCENERENDERPASS_HPP
#define SCENERENDERPASS_HPP

#include "RenderpassBase.hpp"

class SceneRenderpass : public RenderpassBase {
private:
    VkPipelineLayout _pipelineLayout;
    VkPipeline _pipeline;
    std::vector<BoundMeshInfo *> _meshes;

    VkShaderModule createShaderModule(const std::string &path);
    void createPipeline();

public:
    SceneRenderpass(const DeviceData &deviceData, VkPipelineLayout pipelineLayout);

    ~SceneRenderpass() override = default;

    void recordCommands(VkCommandBuffer commandBuffer, uint32_t framebufferIdx, VkRect2D renderArea) override;

    void createRenderpass() override;

    void addMesh(BoundMeshInfo *mesh);
    void removeMesh(BoundMeshInfo *mesh);
};

#endif // SCENERENDERPASS_HPP
