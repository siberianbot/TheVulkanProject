#ifndef RENDERING_RENDERERALLOCATOR_HPP
#define RENDERING_RENDERERALLOCATOR_HPP

#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "src/Types/Vertex.hpp"

class RenderingDevice;
class VulkanObjectsAllocator;
class CommandExecutor;
class BufferObject;
class ImageObject;
class ShaderObject;

class RendererAllocator {
private:
    std::shared_ptr<RenderingDevice> _renderingDevice;
    std::shared_ptr<VulkanObjectsAllocator> _vulkanObjectsAllocator;
    std::shared_ptr<CommandExecutor> _commandExecutor;

    void uploadBuffer(const std::shared_ptr<BufferObject> &targetBuffer, uint64_t size, const void *data);
    void uploadImage(const std::shared_ptr<ImageObject> &targetImage, uint32_t width, uint32_t height, uint32_t size,
                     const std::vector<void *> &data);

public:
    RendererAllocator(const std::shared_ptr<RenderingDevice> &renderingDevice,
                      const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator,
                      const std::shared_ptr<CommandExecutor> &commandExecutor);

    std::shared_ptr<BufferObject> uploadVertices(const std::vector<Vertex> &vertices);
    std::shared_ptr<BufferObject> uploadIndices(const std::vector<uint32_t> &indices);

    std::shared_ptr<ImageObject> uploadImage(uint32_t width, uint32_t height, uint32_t size, void *data);
    std::shared_ptr<ImageObject> uploadCubeImage(uint32_t width, uint32_t height, uint32_t size,
                                                 const std::array<void *, 6> &data);

    ShaderObject *uploadShaderBinary(const std::vector<char> &binary);

    static std::shared_ptr<RendererAllocator> create(
            const std::shared_ptr<RenderingDevice> &renderingDevice,
            const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator,
            const std::shared_ptr<CommandExecutor> &commandExecutor);
};

#endif // RENDERING_RENDERERALLOCATOR_HPP
