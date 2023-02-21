#ifndef RENDERING_RENDERPASSES_SCENERENDERPASS_HPP
#define RENDERING_RENDERPASSES_SCENERENDERPASS_HPP

#include "RenderpassBase.hpp"

#include "Mesh.hpp"
#include "Texture.hpp"

#include "Rendering/Common.hpp"

class Engine;
class CommandExecutor;
class RenderingObjectsFactory;
class BufferObject;
class DescriptorSetObject;
class ImageObject;
class RenderingLayoutObject;

struct BoundMeshInfo {
    BufferObject *vertexBuffer;
    BufferObject *indexBuffer;
    ImageObject *texture;
    glm::mat4 model;
    uint32_t indicesCount;
    DescriptorSetObject *descriptorSet;
};

class SceneRenderpass : public RenderpassBase {
private:
    RenderingObjectsFactory *_renderingObjectsFactory;
    RenderingLayoutObject *_renderingLayoutObject;
    Engine *_engine;
    CommandExecutor *_commandExecutor;

    VkPipeline _pipeline = VK_NULL_HANDLE;
    std::vector<BoundMeshInfo *> _meshes;

    VkSampler _textureSampler;

    BufferObject *uploadVertices(const std::vector<Vertex> &vertices);
    BufferObject *uploadIndices(const std::vector<uint32_t> &indices);
    ImageObject *uploadTexture(const std::string &texturePath);

public:
    SceneRenderpass(RenderingDevice *renderingDevice, Swapchain *swapchain,
                    RenderingObjectsFactory *renderingObjectsFactory, Engine *engine,
                    CommandExecutor *commandExecutor);
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
