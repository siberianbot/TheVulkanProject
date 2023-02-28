#ifndef RENDERING_BUILDERS_SUBPASSBUILDER_HPP
#define RENDERING_BUILDERS_SUBPASSBUILDER_HPP

#include <vector>

#include <vulkan/vulkan.hpp>

class SubpassBuilder {
private:
    std::vector<VkAttachmentReference> _inputAttachments;
    std::vector<VkAttachmentReference> _colorAttachments;
    std::vector<VkAttachmentReference> _resolveAttachments;
    VkAttachmentReference *_depthAttachment = nullptr;

    static VkAttachmentReference *toPtrArray(const std::vector<VkAttachmentReference> &vector);

public:
    SubpassBuilder &withInputAttachment(uint32_t idx, VkImageLayout layout);
    SubpassBuilder &withColorAttachment(uint32_t idx, VkImageLayout layout);
    SubpassBuilder &withResolveAttachment(uint32_t idx, VkImageLayout layout);
    SubpassBuilder &withDepthAttachment(uint32_t idx, VkImageLayout layout);

    VkSubpassDescription build();
};

#endif // RENDERING_BUILDERS_SUBPASSBUILDER_HPP
