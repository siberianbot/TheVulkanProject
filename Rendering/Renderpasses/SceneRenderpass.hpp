#ifndef RENDERING_RENDERPASSES_SCENERENDERPASS_HPP
#define RENDERING_RENDERPASSES_SCENERENDERPASS_HPP

#include "RenderpassBase.hpp"

#include "RendererTypes.hpp"
#include "Mesh.hpp"
#include "Texture.hpp"

class Engine;
class RenderingObjectsFactory;
class VulkanCommandExecutor;

class SceneRenderpass : public RenderpassBase {
private:
    RenderingObjectsFactory *_renderingObjectsFactory;
    Engine *_engine;
    VulkanCommandExecutor *_commandExecutor;

    VkPipeline _pipeline = VK_NULL_HANDLE;
    std::vector<BoundMeshInfo *> _meshes;

    // TODO: REFACTOR!!!
    std::array<BufferObject *, VK_MAX_INFLIGHT_FRAMES> uniformBuffers;
    VkSampler textureSampler;
    VkDescriptorPool descriptorPool;
    VkPipelineLayout _pipelineLayout;
    VkDescriptorSetLayout descriptorSetLayout;

    UniformBufferObject ubo = {};

    void initUniformBuffers();
    void initTextureSampler();
    void initDescriptors();
    void initLayouts();

    BufferObject *uploadVertices(const std::vector<Vertex> &vertices);
    BufferObject *uploadIndices(const std::vector<uint32_t> &indices);
    ImageObject *uploadTexture(const std::string &texturePath);
    std::array<VkDescriptorSet, VK_MAX_INFLIGHT_FRAMES> initDescriptorSets(VkImageView textureImageView);

public:
    SceneRenderpass(RenderingDevice *renderingDevice, Swapchain *swapchain,
                    RenderingObjectsFactory *renderingObjectsFactory, Engine *engine,
                    VulkanCommandExecutor *commandExecutor);
    ~SceneRenderpass() override;

    void recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                        uint32_t frameIdx, uint32_t imageIdx) override;

    void initRenderpass() override;
    void destroyRenderpass() override;

    void addMesh(BoundMeshInfo *mesh);
    void removeMesh(BoundMeshInfo *mesh);

    BoundMeshInfo *uploadMesh(const Mesh &mesh, const Texture &texture);
    void freeMesh(BoundMeshInfo *meshInfo);
};

#endif // RENDERING_RENDERPASSES_SCENERENDERPASS_HPP