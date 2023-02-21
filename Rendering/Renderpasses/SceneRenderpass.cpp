#include "SceneRenderpass.hpp"

#include <fstream>

#include "Engine.hpp"
#include "Mesh.hpp"
#include "Rendering/Common.hpp"
#include "Rendering/RenderingDevice.hpp"
#include "Rendering/Swapchain.hpp"
#include "Rendering/CommandExecutor.hpp"
#include "Rendering/RenderpassBuilder.hpp"
#include "Rendering/PipelineBuilder.hpp"
#include "Rendering/PhysicalDevice.hpp"
#include "Rendering/Objects/BufferObject.hpp"
#include "Rendering/Objects/ImageObject.hpp"
#include "Rendering/Objects/DescriptorSetObject.hpp"
#include "Rendering/Objects/RenderingLayoutObject.hpp"
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

SceneRenderpass::SceneRenderpass(RenderingDevice *renderingDevice, Swapchain *swapchain,
                                 RenderingObjectsFactory *renderingObjectsFactory, Engine *engine,
                                 CommandExecutor *commandExecutor)
        : RenderpassBase(renderingDevice, swapchain),
          _renderingObjectsFactory(renderingObjectsFactory),
          _engine(engine),
          _commandExecutor(commandExecutor) {
    this->_textureSampler = this->_renderingDevice->createSampler();
    this->_renderingLayoutObject = this->_renderingObjectsFactory->createRenderingLayoutObject();
}

void SceneRenderpass::recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                                     uint32_t frameIdx, uint32_t imageIdx) {
    VkExtent2D extent = this->_swapchain->getSwapchainExtent();
    VkPipelineLayout pipelineLayout = this->_renderingLayoutObject->getPipelineLayout();

    SceneData *sceneData = this->_renderingLayoutObject->getSceneData(frameIdx);
    sceneData->view = this->_engine->camera().view();
    sceneData->projection = glm::perspective(glm::radians(45.0f), extent.width / (float) extent.height, 0.1f, 100.0f);
    sceneData->projection[1][1] *= -1;

    VkDescriptorSet sceneDataDescriptorSet = this->_renderingLayoutObject->getSceneDataDescriptorSetObject()
            ->getDescriptorSet(frameIdx);

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

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                            0, 1, &sceneDataDescriptorSet, 0, nullptr);

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
        VkDescriptorSet descriptorSet = mesh->descriptorSet->getDescriptorSet(frameIdx);

        MeshConstants constants = {
                .model = mesh->model
        };
        vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshConstants),
                           &constants);

        VkBuffer vertexBuffer = mesh->vertexBuffer->getHandle();
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);
        vkCmdBindIndexBuffer(commandBuffer, mesh->indexBuffer->getHandle(), 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                                1, 1, &descriptorSet, 0, nullptr);

        vkCmdDrawIndexed(commandBuffer, mesh->indicesCount, 1, 0, 0, idx++);
    }

    vkCmdEndRenderPass(commandBuffer);
}

void SceneRenderpass::initRenderpass() {
    this->_renderpass = RenderpassBuilder(this->_renderingDevice)
            .load()
            .build();

    this->_pipeline = PipelineBuilder(this->_renderingDevice, this->_renderpass,
                                      this->_renderingLayoutObject->getPipelineLayout())
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
    // TODO: release meshes

    delete this->_renderingLayoutObject;
    this->_renderingDevice->destroySampler(this->_textureSampler);
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

BoundMeshInfo *SceneRenderpass::uploadMesh(const Mesh &mesh, const Texture &texture) {
    ImageObject *textureData = uploadTexture(texture.path);

    auto boundMeshInfo = new BoundMeshInfo{
            .vertexBuffer = uploadVertices(mesh.vertices),
            .indexBuffer = uploadIndices(mesh.indices),
            .texture = textureData,
            .model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f)),
            .indicesCount = static_cast<uint32_t>(mesh.indices.size()),
            .descriptorSet = this->_renderingLayoutObject->createMeshDataDescriptor(this->_textureSampler,
                                                                                    textureData->getImageViewHandle())
    };

    this->addMesh(boundMeshInfo);

    return boundMeshInfo;
}

void SceneRenderpass::freeMesh(BoundMeshInfo *meshInfo) {
    this->_renderingDevice->waitIdle();

    delete meshInfo->descriptorSet;
    delete meshInfo->texture;
    delete meshInfo->vertexBuffer;
    delete meshInfo->indexBuffer;

    this->removeMesh(meshInfo);
}
