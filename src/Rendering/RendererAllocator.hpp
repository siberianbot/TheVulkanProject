#ifndef RENDERING_RENDERERALLOCATOR_HPP
#define RENDERING_RENDERERALLOCATOR_HPP

#include <array>
#include <cstdint>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "src/Types/Vertex.hpp"

class RenderingDevice;
class CommandExecutor;
class BufferObject;
class ImageObject;
class ShaderObject;

class RendererAllocator {
private:
    RenderingDevice *_renderingDevice;
    CommandExecutor *_commandExecutor;

    BufferObject *uploadBuffer(uint64_t size, const void *data, VkBufferUsageFlags usage);
    ImageObject *uploadImage(uint32_t width, uint32_t height, uint32_t size, VkImageCreateFlags flags,
                             const std::vector<void *> &data);

public:
    RendererAllocator(RenderingDevice *renderingDevice, CommandExecutor *commandExecutor);

    BufferObject *uploadVertices(const std::vector<Vertex> &vertices);
    BufferObject *uploadIndices(const std::vector<uint32_t> &indices);

    ImageObject *uploadImage(uint32_t width, uint32_t height, uint32_t size, void *data);
    ImageObject *uploadCubeImage(uint32_t width, uint32_t height, uint32_t size, const std::array<void *, 6> &data);

    ShaderObject *uploadShaderBinary(const std::vector<char> &binary);
};

#endif // RENDERING_RENDERERALLOCATOR_HPP
