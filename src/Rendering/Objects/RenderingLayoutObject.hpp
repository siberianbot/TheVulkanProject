#ifndef RENDERING_OBJECTS_RENDERINGLAYOUTOBJECT_HPP
#define RENDERING_OBJECTS_RENDERINGLAYOUTOBJECT_HPP

#include <glm/mat4x4.hpp>
#include <vulkan/vulkan.hpp>

#include "src/Rendering/Common.hpp"

class RenderingDevice;
class RenderingObjectsFactory;
class BufferObject;
class DescriptorSetObject;

struct SceneData {
    glm::mat4 view;
    glm::mat4 projection;
};

struct MeshConstants {
    glm::mat4 model;
};

class RenderingLayoutObject {
private:
    RenderingDevice *_renderingDevice;
    RenderingObjectsFactory *_renderingObjectsFactory;

    VkDescriptorPool _sceneDataDescriptorPool;
    VkDescriptorSetLayout _sceneDataDescriptorSetLayout;
    DescriptorSetObject *_sceneDataDescriptorSetObject;
    std::array<BufferObject *, MAX_INFLIGHT_FRAMES> _sceneDataBuffers;
    std::array<SceneData *, MAX_INFLIGHT_FRAMES> _sceneDataMapped;
    VkDescriptorPool _meshDataDescriptorPool;
    VkDescriptorSetLayout _meshDataDescriptorSetLayout;
    VkPipelineLayout _pipelineLayout;

public:
    RenderingLayoutObject(RenderingDevice *renderingDevice, RenderingObjectsFactory *renderingObjectsFactory,
                          VkDescriptorPool sceneDataDescriptorPool, VkDescriptorSetLayout sceneDataDescriptorSetLayout,
                          VkDescriptorPool meshDataDescriptorPool, VkDescriptorSetLayout meshDataDescriptorSetLayout,
                          VkPipelineLayout pipelineLayout);

    ~RenderingLayoutObject();

    [[nodiscard]] DescriptorSetObject *getSceneDataDescriptorSetObject() const {
        return this->_sceneDataDescriptorSetObject;
    }

    [[nodiscard]] SceneData *getSceneData(uint32_t frameIdx) const {
        return this->_sceneDataMapped[frameIdx];
    }

    [[nodiscard]] VkPipelineLayout getPipelineLayout() const {
        return this->_pipelineLayout;
    }

    [[nodiscard]] DescriptorSetObject *createMeshDataDescriptor(VkSampler textureSampler,
                                                                VkImageView textureImageView);
};


#endif // RENDERING_OBJECTS_RENDERINGLAYOUTOBJECT_HPP
