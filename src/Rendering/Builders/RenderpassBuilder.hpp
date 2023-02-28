#ifndef RENDERING_BUILDERS_RENDERPASSBUILDER_HPP
#define RENDERING_BUILDERS_RENDERPASSBUILDER_HPP

#include <functional>
#include <optional>
#include <vector>

#include <vulkan/vulkan.hpp>

class RenderingDevice;
class AttachmentBuilder;
class SubpassBuilder;

using AddAttachmentFunc = std::function<void(AttachmentBuilder&)>;
using AddSubpassFunc = std::function<void(SubpassBuilder&)>;

class RenderpassBuilder {
private:
    RenderingDevice *_renderingDevice;

    std::vector<VkAttachmentDescription> _attachments;
    std::vector<VkSubpassDescription> _subpasses;
    std::vector<VkSubpassDependency> _dependencies;

public:
    explicit RenderpassBuilder(RenderingDevice *renderingDevice);
    ~RenderpassBuilder();

    RenderpassBuilder &addAttachment(AddAttachmentFunc func);
    RenderpassBuilder &addSubpass(AddSubpassFunc func);
    RenderpassBuilder &addSubpassDependency(uint32_t srcSubpass, uint32_t dstSubpass,
                                            VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                            VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask);

    VkRenderPass build();
};

#endif // RENDERING_BUILDERS_RENDERPASSBUILDER_HPP
