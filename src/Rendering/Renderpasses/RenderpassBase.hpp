#ifndef RENDERING_RENDERPASSES_RENDERPASSBASE_HPP
#define RENDERING_RENDERPASSES_RENDERPASSBASE_HPP

#include <memory>
#include <vector>

#include <vulkan/vulkan.hpp>

class RenderingDevice;
class ImageViewObject;

class RenderpassBase {
protected:
    std::shared_ptr<RenderingDevice> _renderingDevice;

    VkRenderPass _renderpass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> _framebuffers;

public:
    explicit RenderpassBase(const std::shared_ptr<RenderingDevice> &renderingDevice);
    virtual ~RenderpassBase() = default;

    virtual void recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                                uint32_t frameIdx, uint32_t imageIdx) = 0;

    virtual void initRenderpass() = 0;
    virtual void destroyRenderpass();

    virtual void createFramebuffers() = 0;
    virtual void destroyFramebuffers();

    // TODO: imageIdx is not used elsewhere
    virtual ImageViewObject *getResultImageView(uint32_t imageIdx) = 0;
};

#endif // RENDERING_RENDERPASSES_RENDERPASSBASE_HPP
