#include "RenderingObjectsFactory.hpp"

#include "Rendering/RenderingDevice.hpp"
#include "Rendering/Objects/BufferObject.hpp"
#include "Rendering/Objects/DescriptorSetObject.hpp"
#include "Rendering/Objects/FenceObject.hpp"
#include "Rendering/Objects/ImageObject.hpp"
#include "Rendering/Objects/RenderingLayoutObject.hpp"
#include "Rendering/Objects/SemaphoreObject.hpp"

RenderingObjectsFactory::RenderingObjectsFactory(RenderingDevice *renderingDevice)
        : _renderingDevice(renderingDevice) {
    //
}

BufferObject *RenderingObjectsFactory::createBufferObject(VkDeviceSize size, VkBufferUsageFlags usage,
                                                          VkMemoryPropertyFlags memoryProperty) {
    VkBuffer buffer = this->_renderingDevice->createBuffer(size, usage);
    VkMemoryRequirements requirements = this->_renderingDevice->getBufferMemoryRequirements(buffer);
    VkDeviceMemory memory = this->_renderingDevice->allocateMemory(requirements, memoryProperty);

    this->_renderingDevice->bindBufferMemory(buffer, memory);

    return new BufferObject(this->_renderingDevice, size, buffer, memory);
}

ImageObject *RenderingObjectsFactory::createImageObject(uint32_t width, uint32_t height, VkFormat format,
                                                        VkImageUsageFlags usage, VkSampleCountFlagBits samples,
                                                        VkMemoryPropertyFlags memoryProperty,
                                                        VkImageAspectFlags aspectMask) {
    VkImage image = this->_renderingDevice->createImage(width, height, format, usage, samples);
    VkMemoryRequirements requirements = this->_renderingDevice->getImageMemoryRequirements(image);
    VkDeviceMemory memory = this->_renderingDevice->allocateMemory(requirements, memoryProperty);

    this->_renderingDevice->bindImageMemory(image, memory);

    VkImageView imageView = this->_renderingDevice->createImageView(image, format, aspectMask);

    return new ImageObject(this->_renderingDevice, image, memory, imageView);
}

FenceObject *RenderingObjectsFactory::createFenceObject(bool signaled) {
    VkFence fence = this->_renderingDevice->createFence(signaled);

    return new FenceObject(this->_renderingDevice, fence);
}

SemaphoreObject *RenderingObjectsFactory::createSemaphoreObject() {
    VkSemaphore semaphore = this->_renderingDevice->createSemaphore();

    return new SemaphoreObject(this->_renderingDevice, semaphore);
}

RenderingLayoutObject *RenderingObjectsFactory::createRenderingLayoutObject() {
    VkDescriptorPool sceneDataDescriptorPool = this->_renderingDevice->createDescriptorPool(
            {
                    VkDescriptorPoolSize{
                            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                            .descriptorCount = MAX_INFLIGHT_FRAMES
                    }
            },
            MAX_INFLIGHT_FRAMES);

    VkDescriptorSetLayout sceneDataDescriptorSetLayout = this->_renderingDevice->createDescriptorSetLayout(
            {
                    VkDescriptorSetLayoutBinding{
                            .binding = 0,
                            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                            .descriptorCount = 1,
                            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                            .pImmutableSamplers = nullptr
                    }
            });

    VkDescriptorPool meshDataDescriptorPool = this->_renderingDevice->createDescriptorPool(
            {
                    VkDescriptorPoolSize{
                            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                            .descriptorCount = MAX_INFLIGHT_FRAMES
                    }
            },
            MAX_MESH_DATA_DESCRIPTOR_SETS);

    VkDescriptorSetLayout meshDataDescriptorSetLayout = this->_renderingDevice->createDescriptorSetLayout(
            {
                    VkDescriptorSetLayoutBinding{
                            .binding = 0,
                            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                            .descriptorCount = 1,
                            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                            .pImmutableSamplers = nullptr
                    }
            });

    VkPipelineLayout pipelineLayout = this->_renderingDevice->createPipelineLayout(
            {
                    sceneDataDescriptorSetLayout,
                    meshDataDescriptorSetLayout
            },
            {
                    VkPushConstantRange{
                            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                            .offset = 0,
                            .size = sizeof(MeshConstants)
                    }
            });

    return new RenderingLayoutObject(this->_renderingDevice, this,
                                     sceneDataDescriptorPool, sceneDataDescriptorSetLayout,
                                     meshDataDescriptorPool, meshDataDescriptorSetLayout,
                                     pipelineLayout);
}

DescriptorSetObject *RenderingObjectsFactory::createDescriptorSetObject(VkDescriptorPool descriptorPool,
                                                                        VkDescriptorSetLayout descriptorSetLayout) {
    std::array<VkDescriptorSet, MAX_INFLIGHT_FRAMES> descriptorSets = this->_renderingDevice->allocateDescriptorSets(
            descriptorPool, descriptorSetLayout);

    return new DescriptorSetObject(this->_renderingDevice, descriptorPool, descriptorSets);
}
