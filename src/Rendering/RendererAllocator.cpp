#include "RendererAllocator.hpp"

#include "src/Rendering/CommandExecution.hpp"
#include "src/Rendering/CommandExecutor.hpp"
#include "src/Rendering/Objects/BufferObject.hpp"
#include "src/Rendering/Objects/ImageObject.hpp"
#include "src/Rendering/Objects/ShaderObject.hpp"

BufferObject *RendererAllocator::uploadBuffer(uint64_t size, const void *data, VkBufferUsageFlags usage) {
    BufferObject *staging = BufferObject::create(this->_renderingDevice, size,
                                                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    memcpy(staging->map(), data, size);

    BufferObject *result = BufferObject::create(this->_renderingDevice, size,
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

ImageObject *RendererAllocator::uploadImage(uint32_t width, uint32_t height, uint32_t size, VkImageCreateFlags flags,
                                            const std::vector<void *> &data) {
    uint32_t count = data.size();
    uint64_t totalSize = size * count;
    BufferObject *staging = BufferObject::create(this->_renderingDevice, totalSize,
                                                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *ptr = staging->map();
    for (uint32_t idx = 0; idx < count; idx++) {
        void *targetPtr = (void *) ((size_t) ptr + size * idx);
        memcpy(targetPtr, data[idx], size);
    }

    ImageObject *image = ImageObject::create(this->_renderingDevice, width, height, count, flags,
                                             VK_FORMAT_R8G8B8A8_SRGB,
                                             VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                             VK_IMAGE_USAGE_SAMPLED_BIT, VK_SAMPLE_COUNT_1_BIT,
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

                VkBufferImageCopy region = {
                        .bufferOffset = 0,
                        .bufferRowLength = width,
                        .bufferImageHeight = height,
                        .imageSubresource = {
                                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                .mipLevel = 0,
                                .baseArrayLayer = 0,
                                .layerCount = count
                        },
                        .imageOffset = {0, 0, 0},
                        .imageExtent = {width, height, 1}
                };

                vkCmdCopyBufferToImage(cmdBuffer, staging->getHandle(), image->getHandle(),
                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

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

    delete staging;

    return image;
}

RendererAllocator::RendererAllocator(RenderingDevice *renderingDevice, CommandExecutor *commandExecutor)
        : _renderingDevice(renderingDevice),
          _commandExecutor(commandExecutor) {
    //
}

BufferObject *RendererAllocator::uploadVertices(const std::vector<Vertex> &vertices) {
    return this->uploadBuffer(sizeof(Vertex) * vertices.size(), vertices.data(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
}

BufferObject *RendererAllocator::uploadIndices(const std::vector<uint32_t> &indices) {
    return this->uploadBuffer(sizeof(uint32_t) * indices.size(), indices.data(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
}

ImageObject *RendererAllocator::uploadImage(uint32_t width, uint32_t height, uint32_t size, void *data) {
    return this->uploadImage(width, height, size, 0, {data});
}

ImageObject *RendererAllocator::uploadCubeImage(uint32_t width, uint32_t height, uint32_t size,
                                                const std::array<void *, 6> &data) {
    return this->uploadImage(width, height, size, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
                             std::vector(data.begin(), data.end()));
}

ShaderObject *RendererAllocator::uploadShaderBinary(const std::vector<char> &binary) {
    return ShaderObject::create(this->_renderingDevice, binary);
}
