#include "RenderingResourcesManager.hpp"

#include "src/Resources/Mesh.hpp"
#include "src/Resources/Texture.hpp"
#include "RenderingObjectsFactory.hpp"
#include "CommandExecutor.hpp"
#include "src/Rendering/Objects/BufferObject.hpp"
#include "src/Rendering/Objects/ImageObject.hpp"

RenderingResourcesManager::RenderingResourcesManager(RenderingObjectsFactory *renderingObjectsFactory,
                                                     CommandExecutor *commandExecutor)
        : _renderingObjectsFactory(renderingObjectsFactory),
          _commandExecutor(commandExecutor) {
    //
}

BufferObject *RenderingResourcesManager::loadBuffer(uint64_t size, const void *data, VkBufferUsageFlags usage) {
    BufferObject *staging = this->_renderingObjectsFactory->createBufferObject(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    memcpy(staging->map(), data, size);

    BufferObject *result = this->_renderingObjectsFactory->createBufferObject(size,
                                                                              VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage,
                                                                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    this->_commandExecutor->beginOneTimeExecution([&](VkCommandBuffer cmdBuffer) {
                VkBufferCopy bufferCopy = {
                        .srcOffset = 0,
                        .dstOffset = 0,
                        .size = size
                };

                vkCmdCopyBuffer(cmdBuffer, staging->getHandle(), result->getHandle(), 1, &bufferCopy);
            })
            .submit(true);

    delete staging;

    return result;
}

ImageObject *RenderingResourcesManager::loadImage(uint32_t width, uint32_t height, VkImageCreateFlags flags,
                                                  const std::vector<Texture *> &textures) {
    uint32_t count = textures.size();

    std::vector<BufferObject *> stagingBuffers(count);

    for (uint32_t idx = 0; idx < count; idx++) {
        Texture *texture = textures[idx];

        stagingBuffers[idx] = this->_renderingObjectsFactory->createBufferObject(texture->size(),
                                                                                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                                                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        memcpy(stagingBuffers[idx]->map(), texture->data(), texture->size());
    }

    ImageObject *image = this->_renderingObjectsFactory->createImageObject(width, height, count, flags,
                                                                           VK_FORMAT_R8G8B8A8_SRGB,
                                                                           VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                                                           VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                                           VK_IMAGE_USAGE_SAMPLED_BIT,
                                                                           VK_SAMPLE_COUNT_1_BIT,
                                                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    this->_commandExecutor->beginOneTimeExecution([&](VkCommandBuffer cmdBuffer) {
                VkImageMemoryBarrier memoryBarrier;

                memoryBarrier = {
                        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                        .pNext = nullptr,
                        .srcAccessMask = 0,
                        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                        .image = image->getHandle(),
                        .subresourceRange = {
                                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                .baseMipLevel = 0,
                                .levelCount = 1,
                                .baseArrayLayer = 0,
                                .layerCount = count
                        }
                };
                vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                     0, 0, nullptr, 0, nullptr, 1, &memoryBarrier);

                for (uint32_t idx = 0; idx < count; idx++) {
                    VkBufferImageCopy bufferImageCopy = {
                            .bufferOffset = 0,
                            .bufferRowLength = 0,
                            .bufferImageHeight = 0,
                            .imageSubresource = {
                                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                    .mipLevel = 0,
                                    .baseArrayLayer = idx,
                                    .layerCount = 1 // ?
                            },
                            .imageOffset = {0, 0, 0},
                            .imageExtent = {width, height, 1}
                    };
                    vkCmdCopyBufferToImage(cmdBuffer, stagingBuffers[idx]->getHandle(), image->getHandle(),
                                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);
                }

                memoryBarrier = {
                        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                        .pNext = nullptr,
                        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
                        .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                        .image = image->getHandle(),
                        .subresourceRange = {
                                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                .baseMipLevel = 0,
                                .levelCount = 1,
                                .baseArrayLayer = 0,
                                .layerCount = count
                        }
                };
                vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                     0, 0, nullptr, 0, nullptr, 1, &memoryBarrier);

            })
            .submit(true);

    for (uint32_t idx = 0; idx < count; idx++) {
        delete stagingBuffers[idx];
    }

    return image;
}

MeshRenderingResource RenderingResourcesManager::loadMesh(Mesh *mesh) {
    return {
            .vertices = loadBuffer(sizeof(Vertex) * mesh->vertices().size(), mesh->vertices().data(),
                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT),
            .indices = loadBuffer(sizeof(uint32_t) * mesh->indices().size(), mesh->indices().data(),
                                  VK_BUFFER_USAGE_INDEX_BUFFER_BIT),
            .count = static_cast<uint32_t>(mesh->indices().size())
    };
}

MeshRenderingResource RenderingResourcesManager::loadMesh(uint32_t count, const Vertex *data) {
    return {
            .vertices = loadBuffer(sizeof(Vertex) * count, reinterpret_cast<const void *>(data),
                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT),
            .indices = nullptr,
            .count = count
    };
}

TextureRenderingResource RenderingResourcesManager::loadTexture(Texture *texture) {
    return {
            .texture = loadImage(texture->width(), texture->height(), 0, {texture})
    };
}

TextureRenderingResource RenderingResourcesManager::loadTextureArray(const std::vector<Texture *> &textures) {
    return {
            .texture = loadImage(textures[0]->width(), textures[0]->height(),
                                 0, textures)
    };
}

TextureRenderingResource RenderingResourcesManager::loadTextureCube(const std::vector<Texture *> &textures) {
    if (textures.size() != 6) {
        throw std::runtime_error("Cube texture should contain 6 textures");
    }

    return {
            .texture = loadImage(textures[0]->width(), textures[0]->height(),
                                 VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, textures)
    };
}

void RenderingResourcesManager::freeMesh(const MeshRenderingResource &meshResource) {
    delete meshResource.vertices;

    if (meshResource.indices != nullptr) {
        delete meshResource.indices;
    }
}

void RenderingResourcesManager::freeTexture(const TextureRenderingResource &textureResource) {
    delete textureResource.texture;
}
