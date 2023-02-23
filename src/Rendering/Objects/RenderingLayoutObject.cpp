#include "RenderingLayoutObject.hpp"

#include "src/Rendering/RenderingDevice.hpp"
#include "src/Rendering/RenderingObjectsFactory.hpp"
#include "BufferObject.hpp"
#include "DescriptorSetObject.hpp"

RenderingLayoutObject::RenderingLayoutObject(RenderingDevice *renderingDevice,
                                             RenderingObjectsFactory *renderingObjectsFactory,
                                             VkDescriptorPool sceneDataDescriptorPool,
                                             VkDescriptorSetLayout sceneDataDescriptorSetLayout,
                                             VkDescriptorPool meshDataDescriptorPool,
                                             VkDescriptorSetLayout meshDataDescriptorSetLayout,
                                             VkPipelineLayout pipelineLayout)
        : _renderingDevice(renderingDevice),
          _renderingObjectsFactory(renderingObjectsFactory),
          _sceneDataDescriptorPool(sceneDataDescriptorPool),
          _sceneDataDescriptorSetLayout(sceneDataDescriptorSetLayout),
          _meshDataDescriptorPool(meshDataDescriptorPool),
          _meshDataDescriptorSetLayout(meshDataDescriptorSetLayout),
          _pipelineLayout(pipelineLayout) {

    this->_sceneDataDescriptorSetObject = this->_renderingObjectsFactory->createDescriptorSetObject(
            this->_sceneDataDescriptorPool,
            this->_sceneDataDescriptorSetLayout);

    std::vector<VkWriteDescriptorSet> writes;
    for (uint32_t idx = 0; idx < MAX_INFLIGHT_FRAMES; idx++) {
        this->_sceneDataBuffers[idx] = this->_renderingObjectsFactory->createBufferObject(
                sizeof(SceneData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        this->_sceneDataMapped[idx] = reinterpret_cast<SceneData *>(this->_sceneDataBuffers[idx]->map());

        VkDescriptorBufferInfo bufferInfo = {
                .buffer = this->_sceneDataBuffers[idx]->getHandle(),
                .offset = 0,
                .range = sizeof(SceneData)
        };

        writes.push_back(VkWriteDescriptorSet{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = this->_sceneDataDescriptorSetObject->getDescriptorSet(idx),
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pImageInfo = nullptr,
                .pBufferInfo = &bufferInfo,
                .pTexelBufferView = nullptr
        });
    }

    this->_renderingDevice->updateDescriptorSets(writes);
}

RenderingLayoutObject::~RenderingLayoutObject() {
    this->_renderingDevice->destroyPipelineLayout(this->_pipelineLayout);

    this->_renderingDevice->destroyDescriptorSetLayout(this->_meshDataDescriptorSetLayout);
    this->_renderingDevice->destroyDescriptorPool(this->_meshDataDescriptorPool);

    delete this->_sceneDataDescriptorSetObject;

    for (BufferObject *buffer: this->_sceneDataBuffers) {
        delete buffer;
    }

    this->_renderingDevice->destroyDescriptorSetLayout(this->_sceneDataDescriptorSetLayout);
    this->_renderingDevice->destroyDescriptorPool(this->_sceneDataDescriptorPool);
}

DescriptorSetObject *RenderingLayoutObject::createMeshDataDescriptor(VkSampler textureSampler,
                                                                     VkImageView textureImageView) {
    DescriptorSetObject *descriptorSetObject = this->_renderingObjectsFactory->createDescriptorSetObject(
            this->_meshDataDescriptorPool,
            this->_meshDataDescriptorSetLayout);

    VkDescriptorImageInfo imageInfo = {
            .sampler = textureSampler,
            .imageView = textureImageView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    std::vector<VkWriteDescriptorSet> writes;
    for (uint32_t idx = 0; idx < MAX_INFLIGHT_FRAMES; idx++) {
        writes.push_back(VkWriteDescriptorSet{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = descriptorSetObject->getDescriptorSet(idx),
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = &imageInfo,
                .pBufferInfo = nullptr,
                .pTexelBufferView = nullptr
        });
    }

    this->_renderingDevice->updateDescriptorSets(writes);

    return descriptorSetObject;
}
