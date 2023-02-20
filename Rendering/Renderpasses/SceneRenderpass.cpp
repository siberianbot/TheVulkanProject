#include "SceneRenderpass.hpp"

#include <fstream>

#include "Engine.hpp"
#include "Mesh.hpp"
#include "VulkanCommon.hpp"
#include "Rendering/Swapchain.hpp"
#include "Rendering/VulkanCommandExecutor.hpp"
#include "Rendering/VulkanRenderpassBuilder.hpp"
#include "Rendering/VulkanPipelineBuilder.hpp"
#include "Rendering/VulkanPhysicalDevice.hpp"
#include "Rendering/RenderingObjectsFactory.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

static constexpr const char *DEFAULT_VERTEX_SHADER = "shaders/default.vert.spv";
static constexpr const char *DEFAULT_FRAGMENT_SHADER = "shaders/default.frag.spv";

void SceneRenderpass::initUniformBuffers() {
    VkDeviceSize uboSize = sizeof(UniformBufferObject);

    for (size_t idx = 0; idx < VK_MAX_INFLIGHT_FRAMES; idx++) {
        this->uniformBuffers[idx] = this->_renderingObjectsFactory
                ->createBufferObject(uboSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }
}

void SceneRenderpass::initTextureSampler() {
    VkSamplerCreateInfo samplerCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .mipmapMode = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .mipLodBias = 0,
            .anisotropyEnable = VK_TRUE,
            .maxAnisotropy = this->_renderingDevice->getPhysicalDevice()->getMaxSamplerAnisotropy(),
            .compareEnable = VK_FALSE,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .minLod = 0,
            .maxLod = 1,
            .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE
    };

    vkEnsure(vkCreateSampler(this->_renderingDevice->getHandle(), &samplerCreateInfo, nullptr, &this->textureSampler));
}

void SceneRenderpass::initDescriptors() {
    std::array<VkDescriptorPoolSize, 2> descriptorPoolSizes = {
            VkDescriptorPoolSize{
                    .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .descriptorCount = VK_MAX_INFLIGHT_FRAMES
            },
            VkDescriptorPoolSize{
                    .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .descriptorCount = VK_MAX_INFLIGHT_FRAMES
            }
    };

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
            .maxSets = 4 * VK_MAX_INFLIGHT_FRAMES,
            .poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size()),
            .pPoolSizes = descriptorPoolSizes.data()
    };

    vkEnsure(vkCreateDescriptorPool(this->_renderingDevice->getHandle(), &descriptorPoolCreateInfo, nullptr,
                                    &this->descriptorPool));
}

void SceneRenderpass::initLayouts() {
    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {
            VkDescriptorSetLayoutBinding{
                    .binding = 0,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                    .pImmutableSamplers = nullptr
            },
            VkDescriptorSetLayoutBinding{
                    .binding = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .pImmutableSamplers = nullptr
            }
    };

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .bindingCount = static_cast<uint32_t>(bindings.size()),
            .pBindings = bindings.data()
    };

    vkEnsure(vkCreateDescriptorSetLayout(this->_renderingDevice->getHandle(), &descriptorSetLayoutCreateInfo, nullptr,
                                         &this->descriptorSetLayout));

    VkPushConstantRange constantRange = {
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .offset = 0,
            .size = sizeof(Constants)
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .setLayoutCount = 1,
            .pSetLayouts = &this->descriptorSetLayout,
            .pushConstantRangeCount = 1,
            .pPushConstantRanges = &constantRange
    };

    vkEnsure(vkCreatePipelineLayout(this->_renderingDevice->getHandle(), &pipelineLayoutInfo, nullptr,
                                    &this->_pipelineLayout));
}

SceneRenderpass::SceneRenderpass(RenderingDevice *renderingDevice, Swapchain *swapchain,
                                 RenderingObjectsFactory *renderingObjectsFactory, Engine *engine,
                                 VulkanCommandExecutor *commandExecutor)
        : RenderpassBase(renderingDevice, swapchain),
          _renderingObjectsFactory(renderingObjectsFactory),
          _engine(engine),
          _commandExecutor(commandExecutor) {
    initUniformBuffers();
    initTextureSampler();
    initDescriptors();
    initLayouts();
}

void SceneRenderpass::recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                                     uint32_t frameIdx, uint32_t imageIdx) {
    VkExtent2D extent = this->_swapchain->getSwapchainExtent();

    ubo.view = this->_engine->camera().view();
    ubo.proj = glm::perspective(glm::radians(45.0f), extent.width / (float) extent.height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;
    memcpy(this->uniformBuffers[frameIdx]->map(), &ubo, sizeof(ubo));

    const VkRenderPassBeginInfo renderPassBeginInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = this->_renderpass,
            .framebuffer = this->_framebuffers[imageIdx],
            .renderArea = renderArea,
            .clearValueCount = 0,
            .pClearValues = nullptr
    };

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_pipeline);

    VkViewport viewport = {
            .x = 0,
            .y = 0,
            .width = (float) renderArea.extent.width,
            .height = (float) renderArea.extent.height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f
    };
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {
            .offset = {0, 0},
            .extent = renderArea.extent
    };
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    int idx = 0;
    for (BoundMeshInfo *mesh: this->_meshes) {
        VkDeviceSize offset = 0;
        VkDescriptorSet descriptorSet = mesh->descriptorSets[frameIdx];

        Constants constants = {
                .model = mesh->model
        };
        vkCmdPushConstants(commandBuffer, this->_pipelineLayout,
                           VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Constants), &constants);

        VkBuffer vertexBuffer = mesh->vertexBuffer->getHandle();
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);
        vkCmdBindIndexBuffer(commandBuffer, mesh->indexBuffer->getHandle(), 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_pipelineLayout,
                                0, 1, &descriptorSet, 0, nullptr);

        vkCmdDrawIndexed(commandBuffer, mesh->indicesCount, 1, 0, 0, idx++);
    }

    vkCmdEndRenderPass(commandBuffer);
}

void SceneRenderpass::initRenderpass() {
    this->_renderpass = VulkanRenderpassBuilder(this->_renderingDevice)
            .load()
            .build();

    this->_pipeline = VulkanPipelineBuilder(this->_renderingDevice, this->_renderpass, this->_pipelineLayout)
            .addVertexShader(DEFAULT_VERTEX_SHADER)
            .addFragmentShader(DEFAULT_FRAGMENT_SHADER)
            .addBinding(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
            .addAttribute(0, 0, offsetof(Vertex, pos), VK_FORMAT_R32G32B32_SFLOAT)
            .addAttribute(0, 1, offsetof(Vertex, color), VK_FORMAT_R32G32B32_SFLOAT)
            .addAttribute(0, 2, offsetof(Vertex, texCoord), VK_FORMAT_R32G32_SFLOAT)
            .build();
}

void SceneRenderpass::destroyRenderpass() {
    vkDestroyPipeline(this->_renderingDevice->getHandle(), this->_pipeline, nullptr);

    RenderpassBase::destroyRenderpass();
}

void SceneRenderpass::addMesh(BoundMeshInfo *mesh) {
    this->_meshes.push_back(mesh);
}

void SceneRenderpass::removeMesh(BoundMeshInfo *mesh) {
    this->_meshes.erase(std::remove(this->_meshes.begin(), this->_meshes.end(), mesh));
}

SceneRenderpass::~SceneRenderpass() {
    vkDestroyPipelineLayout(this->_renderingDevice->getHandle(), this->_pipelineLayout, nullptr);

    vkDestroyDescriptorSetLayout(this->_renderingDevice->getHandle(), this->descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(this->_renderingDevice->getHandle(), this->descriptorPool, nullptr);

    vkDestroySampler(this->_renderingDevice->getHandle(), this->textureSampler, nullptr);

    for (uint32_t frameIdx = 0; frameIdx < VK_MAX_INFLIGHT_FRAMES; frameIdx++) {
        delete this->uniformBuffers[frameIdx];
    }
}

BufferObject *SceneRenderpass::uploadVertices(const std::vector<Vertex> &vertices) {
    // TODO: use staging buffer to restrict usage of GPU memory by CPU

    VkDeviceSize size = sizeof(vertices[0]) * vertices.size();

    BufferObject *buffer = this->_renderingObjectsFactory->createBufferObject(size,
                                                                              VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                                                              VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                                                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    memcpy(buffer->map(), vertices.data(), size);
    buffer->unmap();

    return buffer;
}

BufferObject *SceneRenderpass::uploadIndices(const std::vector<uint32_t> &indices) {
    // TODO: use staging buffer to restrict usage of GPU memory by CPU

    VkDeviceSize size = sizeof(indices[0]) * indices.size();

    BufferObject *buffer = this->_renderingObjectsFactory->createBufferObject(size,
                                                                              VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                                                              VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                                                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    memcpy(buffer->map(), indices.data(), size);
    buffer->unmap();

    return buffer;
}

ImageObject *SceneRenderpass::uploadTexture(const std::string &texturePath) {
    int width, height, channels;
    stbi_uc *pixels = stbi_load(texturePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    VkDeviceSize imageSize = width * height * 4;

    BufferObject *stagingBuffer = this->_renderingObjectsFactory->createBufferObject(imageSize,
                                                                                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                                                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    memcpy(stagingBuffer->map(), pixels, static_cast<size_t>(imageSize));
    stagingBuffer->unmap();
    stbi_image_free(pixels);

    ImageObject *image = this->_renderingObjectsFactory->createImageObject(width, height, VK_FORMAT_R8G8B8A8_SRGB,
                                                                           VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                                                           VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                                           VK_IMAGE_USAGE_SAMPLED_BIT,
                                                                           VK_SAMPLE_COUNT_1_BIT,
                                                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                                           VK_IMAGE_ASPECT_COLOR_BIT);

    this->_commandExecutor->beginOneTimeExecution(
                    [&image, &width, &height, &stagingBuffer](VkCommandBuffer cmdBuffer) {
                        VkImageMemoryBarrier imageMemoryBarrier = {
                                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                                .pNext = nullptr,
                                .srcAccessMask = 0,
                                .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                .image = image->getImageHandle(),
                                .subresourceRange = {
                                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                        .baseMipLevel = 0,
                                        .levelCount = 1,
                                        .baseArrayLayer = 0,
                                        .layerCount = 1
                                }
                        };
                        vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                                             0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

                        VkBufferImageCopy bufferImageCopy = {
                                .bufferOffset = 0,
                                .bufferRowLength = 0,
                                .bufferImageHeight = 0,
                                .imageSubresource = {
                                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                        .mipLevel = 0,
                                        .baseArrayLayer = 0,
                                        .layerCount = 1
                                },
                                .imageOffset = {0, 0, 0},
                                .imageExtent = {(uint32_t) width, (uint32_t) height, 1}
                        };
                        vkCmdCopyBufferToImage(cmdBuffer, stagingBuffer->getHandle(), image->getImageHandle(),
                                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                                               &bufferImageCopy);

                        VkImageMemoryBarrier anotherImageMemoryBarrier = {
                                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                                .pNext = nullptr,
                                .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                                .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
                                .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                .image = image->getImageHandle(),
                                .subresourceRange = {
                                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                        .baseMipLevel = 0,
                                        .levelCount = 1,
                                        .baseArrayLayer = 0,
                                        .layerCount = 1
                                }
                        };
                        vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                             0, 0, nullptr, 0, nullptr, 1, &anotherImageMemoryBarrier);

                    })
            .submit(true);

    delete stagingBuffer;

    return image;
}

std::array<VkDescriptorSet, VK_MAX_INFLIGHT_FRAMES> SceneRenderpass::initDescriptorSets(VkImageView textureImageView) {
    std::vector<VkDescriptorSetLayout> layouts(VK_MAX_INFLIGHT_FRAMES, this->descriptorSetLayout);
    std::array<VkDescriptorSet, VK_MAX_INFLIGHT_FRAMES> descriptorSets = {};

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = this->descriptorPool,
            .descriptorSetCount = static_cast<uint32_t>(descriptorSets.size()),
            .pSetLayouts = layouts.data()
    };

    vkEnsure(vkAllocateDescriptorSets(this->_renderingDevice->getHandle(), &descriptorSetAllocateInfo,
                                      descriptorSets.data()));

    for (uint32_t idx = 0; idx < VK_MAX_INFLIGHT_FRAMES; idx++) {
        VkDescriptorBufferInfo bufferInfo = {
                .buffer = this->uniformBuffers[idx]->getHandle(),
                .offset = 0,
                .range = sizeof(UniformBufferObject)
        };

        VkDescriptorImageInfo imageInfo = {
                .sampler = this->textureSampler,
                .imageView = textureImageView,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };

        std::array<VkWriteDescriptorSet, 2> writes = {
                VkWriteDescriptorSet{
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .pNext = nullptr,
                        .dstSet = descriptorSets[idx],
                        .dstBinding = 0,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        .pImageInfo = nullptr,
                        .pBufferInfo = &bufferInfo,
                        .pTexelBufferView = nullptr
                },
                VkWriteDescriptorSet{
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .pNext = nullptr,
                        .dstSet = descriptorSets[idx],
                        .dstBinding = 1,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        .pImageInfo = &imageInfo,
                        .pBufferInfo = nullptr,
                        .pTexelBufferView = nullptr
                }
        };

        vkUpdateDescriptorSets(this->_renderingDevice->getHandle(), static_cast<uint32_t>(writes.size()), writes.data(),
                               0, nullptr);
    }

    return descriptorSets;
}

BoundMeshInfo *SceneRenderpass::uploadMesh(const Mesh &mesh, const Texture &texture) {
    ImageObject *textureData = uploadTexture(texture.path);

    auto boundMeshInfo = new BoundMeshInfo{
            .vertexBuffer = uploadVertices(mesh.vertices),
            .indexBuffer = uploadIndices(mesh.indices),
            .texture = textureData,
            .model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f)),
            .indicesCount = static_cast<uint32_t>(mesh.indices.size()),
            .descriptorSets = initDescriptorSets(textureData->getImageViewHandle())
    };

    this->addMesh(boundMeshInfo);

    return boundMeshInfo;
}

void SceneRenderpass::freeMesh(BoundMeshInfo *meshInfo) {
    vkEnsure(vkDeviceWaitIdle(this->_renderingDevice->getHandle()));

    vkFreeDescriptorSets(this->_renderingDevice->getHandle(), this->descriptorPool,
                         static_cast<uint32_t>(meshInfo->descriptorSets.size()),
                         meshInfo->descriptorSets.data());

    delete meshInfo->texture;
    delete meshInfo->vertexBuffer;
    delete meshInfo->indexBuffer;

    this->removeMesh(meshInfo);
}
