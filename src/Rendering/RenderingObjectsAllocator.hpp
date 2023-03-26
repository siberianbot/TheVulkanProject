#ifndef RENDERING_RENDERERINGOBJECTSALLOCATOR_HPP
#define RENDERING_RENDERERINGOBJECTSALLOCATOR_HPP

#include <array>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "src/Types/Vertex.hpp"

class RenderingDevice;
class VulkanObjectsAllocator;
class CommandExecutor;
class BufferObject;
class ImageObject;
class ImageViewObject;
class ShaderObject;

class RenderingObjectsAllocator {
private:
    std::shared_ptr<RenderingDevice> _renderingDevice;
    std::shared_ptr<VulkanObjectsAllocator> _vulkanObjectsAllocator;
    std::shared_ptr<CommandExecutor> _commandExecutor;

    std::map<std::string, std::shared_ptr<ImageViewObject>> _imageViews;

    void uploadBuffer(const std::shared_ptr<BufferObject> &targetBuffer, uint64_t size, const void *data);
    void uploadImage(const std::shared_ptr<ImageObject> &targetImage, uint32_t width, uint32_t height, uint32_t size,
                     const std::vector<void *> &data);

public:
    RenderingObjectsAllocator(const std::shared_ptr<RenderingDevice> &renderingDevice,
                              const std::shared_ptr<VulkanObjectsAllocator> &vulkanObjectsAllocator,
                              const std::shared_ptr<CommandExecutor> &commandExecutor);

    void destroy();

    std::shared_ptr<BufferObject> uploadVertices(const std::vector<Vertex> &vertices);
    std::shared_ptr<BufferObject> uploadIndices(const std::vector<uint32_t> &indices);

    std::shared_ptr<ImageObject> uploadImage(uint32_t width, uint32_t height, uint32_t size, void *data);
    std::shared_ptr<ImageObject> uploadCubeImage(uint32_t width, uint32_t height, uint32_t size,
                                                 const std::array<void *, 6> &data);

    std::shared_ptr<ShaderObject> uploadShaderBinary(const std::vector<char> &binary);
};

#endif // RENDERING_RENDERERINGOBJECTSALLOCATOR_HPP
