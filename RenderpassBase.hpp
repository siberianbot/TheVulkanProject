#ifndef RENDERPASSBASE_HPP
#define RENDERPASSBASE_HPP

#include <vector>

#include "AnotherVulkanTypes.hpp"

class RenderpassBase {
protected:
    RenderingDevice _renderingDevice;

    VkRenderPass _renderpass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> _framebuffers;

public:
    explicit RenderpassBase(const RenderingDevice &renderingDevice);
    virtual ~RenderpassBase();

    virtual void recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                                uint32_t frameIdx, uint32_t imageIdx) = 0;

    virtual void initRenderpass() = 0;
    virtual void destroyRenderpass();

    virtual void createFramebuffers(uint32_t width, uint32_t height, uint32_t targetImagesCount,
                                    const std::vector<std::vector<VkImageView>> &targetImageGroups);
    void destroyFramebuffers();
};

#endif // RENDERPASSBASE_HPP
