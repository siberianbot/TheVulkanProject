#include "RenderingObjectsAllocator.hpp"

#include "src/Rendering/CommandExecution.hpp"
#include "src/Rendering/CommandExecutor.hpp"
#include "src/Rendering/Builders/BufferObjectBuilder.hpp"
#include "src/Rendering/Builders/ImageObjectBuilder.hpp"
#include "src/Rendering/Objects/BufferObject.hpp"
#include "src/Rendering/Objects/ImageObject.hpp"
#include "src/Rendering/Objects/ShaderObject.hpp"

void RenderingObjectsAllocator::uploadBuffer(const std::shared_ptr<BufferObject> &targetBuffer, uint64_t size,
                                             const void *data) {
    std::shared_ptr<BufferObject> staging = BufferObjectBuilder(this->_renderingDevice, this->_vulkanObjectsAllocator)
            .withSize(size)
            .withUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
            .hostAvailable()
            .build();

    memcpy(staging->map(), data, size);

    this->_commandExecutor->beginOneTimeExecution([&](VkCommandBuffer cmdBuffer) {
                VkBufferCopy bufferCopy = {
                        .srcOffset = 0,
                        .dstOffset = 0,
                        .size = size
                };

                vkCmdCopyBuffer(cmdBuffer, staging->getHandle(), targetBuffer->getHandle(), 1, &bufferCopy);
            })
            .submit(true);

    staging->destroy();
}

void RenderingObjectsAllocator::uploadImage(const std::shared_ptr<ImageObject> &targetImage,
                                            uint32_t width, uint32_t height,
                                            uint32_t size, const std::vector<void *> &data) {
    uint32_t count = data.size();
    uint64_t totalSize = size * count;
    std::shared_ptr<BufferObject> staging = BufferObjectBuilder(this->_renderingDevice, this->_vulkanObjectsAllocator)
            .withSize(totalSize)
            .withUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
            .hostAvailable()
            .build();

    void *ptr = staging->map();
    for (uint32_t idx = 0; idx < count; idx++) {
        void *targetPtr = (void *) ((size_t) ptr + size * idx);
        memcpy(targetPtr, data[idx], size);
    }

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
                        .image = targetImage->getHandle(),
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

                vkCmdCopyBufferToImage(cmdBuffer, staging->getHandle(), targetImage->getHandle(),
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
                        .image = targetImage->getHandle(),
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

    staging->destroy();
}

RenderingObjectsAllocator::RenderingObjectsAllocator(const std::shared_ptr<RenderingDevice> &renderingDevice,
                                                     const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator,
                                                     const std::shared_ptr<CommandExecutor> &commandExecutor)
        : _renderingDevice(renderingDevice),
          _vulkanObjectsAllocator(vulkanObjectsAllocator),
          _commandExecutor(commandExecutor) {
    //
}

std::shared_ptr<BufferObject> RenderingObjectsAllocator::uploadVertices(const std::vector<Vertex> &vertices) {
    VkDeviceSize size = sizeof(Vertex) * vertices.size();
    std::shared_ptr<BufferObject> buffer = BufferObjectBuilder(this->_renderingDevice, this->_vulkanObjectsAllocator)
            .withSize(size)
            .withUsage(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
            .deviceLocal()
            .build();

    this->uploadBuffer(buffer, size, vertices.data());

    return buffer;
}

std::shared_ptr<BufferObject> RenderingObjectsAllocator::uploadIndices(const std::vector<uint32_t> &indices) {
    VkDeviceSize size = sizeof(uint32_t) * indices.size();
    std::shared_ptr<BufferObject> buffer = BufferObjectBuilder(this->_renderingDevice, this->_vulkanObjectsAllocator)
            .withSize(size)
            .withUsage(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
            .deviceLocal()
            .build();

    this->uploadBuffer(buffer, size, indices.data());

    return buffer;
}

std::shared_ptr<ImageObject> RenderingObjectsAllocator::uploadImage(uint32_t width, uint32_t height, uint32_t size,
                                                                    void *data) {
    std::shared_ptr<ImageObject> image = ImageObjectBuilder(this->_renderingDevice, this->_vulkanObjectsAllocator)
            .withExtent(width, height)
            .withFormat(VK_FORMAT_R8G8B8A8_SRGB)
            .withUsage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .build();

    this->uploadImage(image, width, height, size, {data});

    return image;
}

std::shared_ptr<ImageObject> RenderingObjectsAllocator::uploadCubeImage(uint32_t width, uint32_t height, uint32_t size,
                                                                        const std::array<void *, 6> &data) {
    std::shared_ptr<ImageObject> image = ImageObjectBuilder(this->_renderingDevice, this->_vulkanObjectsAllocator)
            .withExtent(width, height)
            .withFormat(VK_FORMAT_R8G8B8A8_SRGB)
            .withUsage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .cubeCompatible()
            .build();

    this->uploadImage(image, width, height, size, std::vector(data.begin(), data.end()));

    return image;
}

std::shared_ptr<ShaderObject> RenderingObjectsAllocator::uploadShaderBinary(const std::vector<char> &binary) {
    return ShaderObject::create(this->_vulkanObjectsAllocator, binary);
}
