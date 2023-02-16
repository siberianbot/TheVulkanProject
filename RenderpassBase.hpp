#ifndef RENDERPASSBASE_HPP
#define RENDERPASSBASE_HPP

#include <vector>

#include <vulkan/vulkan.hpp>

#include "RendererTypes.hpp"

enum RenderpassFlags {
    RENDERPASS_NONE = 0,
    RENDERPASS_FIRST = 0b01,
    RENDERPASS_LAST = 0b10,
};

class RenderpassBase {
protected:
    RenderpassFlags _flags;
    DeviceData _deviceData;
    VkRenderPass _renderpass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> _framebuffers;

public:
    RenderpassBase(const RenderpassFlags &flags, const DeviceData &deviceData);

    virtual ~RenderpassBase();

    virtual void recordCommands(VkCommandBuffer commandBuffer, uint32_t framebufferIdx, VkRect2D renderArea) = 0;

    virtual void createRenderpass();
    virtual void createFramebuffers(uint32_t width, uint32_t height, uint32_t targetImagesCount,
                                    const std::vector<std::vector<VkImageView>> &targetImageGroups);

    void destroyFramebuffers();
};

#endif // RENDERPASSBASE_HPP
