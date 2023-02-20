#include "RenderpassBuilder.hpp"

#include "Rendering/Common.hpp"
#include "Rendering/PhysicalDevice.hpp"
#include "Rendering/RenderingDevice.hpp"

RenderpassBuilder::RenderpassBuilder(RenderingDevice *renderingDevice)
        : _renderingDevice(renderingDevice) {
    //
}

RenderpassBuilder &RenderpassBuilder::clear() {
    this->_loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

    this->_colorAttachment = Attachment{
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    this->_depthAttachment = Attachment{
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    return *this;
}

RenderpassBuilder &RenderpassBuilder::load() {
    this->_loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

    this->_colorAttachment = Attachment{
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    this->_depthAttachment = Attachment{
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    return *this;
}

RenderpassBuilder &RenderpassBuilder::addResolveAttachment() {
    this->_resolveAttachment = Attachment{
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    return *this;
}

VkRenderPass RenderpassBuilder::build() {
    if (!this->_loadOp.has_value()) {
        throw std::runtime_error("Load operator is required");
    }

    if (!this->_colorAttachment.has_value() ||
        !this->_depthAttachment.has_value()) {
        throw std::runtime_error("Color and depth attachments are required");
    }

    std::vector<VkAttachmentDescription> attachments = {
            VkAttachmentDescription{
                    .flags = 0,
                    .format = this->_renderingDevice->getPhysicalDevice()->getColorFormat(),
                    .samples = this->_renderingDevice->getPhysicalDevice()->getMsaaSamples(),
                    .loadOp = this->_loadOp.value(),
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .initialLayout = this->_colorAttachment.value().initialLayout,
                    .finalLayout = this->_colorAttachment.value().finalLayout
            },
            VkAttachmentDescription{
                    .flags = 0,
                    .format = this->_renderingDevice->getPhysicalDevice()->getDepthFormat(),
                    .samples = this->_renderingDevice->getPhysicalDevice()->getMsaaSamples(),
                    .loadOp = this->_loadOp.value(),
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .initialLayout = this->_depthAttachment.value().initialLayout,
                    .finalLayout = this->_depthAttachment.value().finalLayout
            }
    };

    const VkAttachmentReference colorReference = {
            .attachment = COLOR_ATTACHMENT_IDX,
            .layout = this->_colorAttachment->layout
    };

    const VkAttachmentReference depthReference = {
            .attachment = DEPTH_ATTACHMENT_IDX,
            .layout = this->_depthAttachment->layout
    };

    std::optional<VkAttachmentReference> resolveReference;
    if (_resolveAttachment.has_value()) {
        attachments.push_back(VkAttachmentDescription{
                .flags = 0,
                .format = this->_renderingDevice->getPhysicalDevice()->getColorFormat(),
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = this->_resolveAttachment.value().initialLayout,
                .finalLayout = this->_resolveAttachment.value().finalLayout
        });

        resolveReference = VkAttachmentReference{
                .attachment = RESOLVE_ATTACHMENT_IDX,
                .layout = this->_resolveAttachment->layout
        };
    }

    const std::array<VkSubpassDependency, 1> dependencies = {
            VkSubpassDependency{
                    .srcSubpass = VK_SUBPASS_EXTERNAL,
                    .dstSubpass = 0,
                    .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    .srcAccessMask = 0,
                    .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                    .dependencyFlags = 0
            }
    };

    const VkSubpassDescription subpass = {
            .flags = 0,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = 0,
            .pInputAttachments = nullptr,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorReference,
            .pResolveAttachments = resolveReference.has_value()
                                   ? &resolveReference.value()
                                   : nullptr,
            .pDepthStencilAttachment = &depthReference,
            .preserveAttachmentCount = 0,
            .pPreserveAttachments = nullptr
    };

    const VkRenderPassCreateInfo renderPassCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = static_cast<uint32_t>(dependencies.size()),
            .pDependencies = dependencies.data()
    };

    VkRenderPass renderpass;
    vkEnsure(vkCreateRenderPass(this->_renderingDevice->getHandle(), &renderPassCreateInfo, nullptr, &renderpass));

    return renderpass;
}
