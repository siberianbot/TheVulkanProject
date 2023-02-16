#include "RenderpassBase.hpp"

#include "VulkanCommon.hpp"

void RenderpassBase::createRenderpass() {
    const bool isFirst = this->_flags & RENDERPASS_FIRST;
    const bool isLast = this->_flags & RENDERPASS_LAST;

    const std::array<VkAttachmentDescription, 2> attachments = {
            VkAttachmentDescription{
                    .flags = 0,
                    .format= this->_deviceData.colorFormat,
                    .samples = this->_deviceData.msaaSamples,
                    .loadOp = isFirst
                              ? VK_ATTACHMENT_LOAD_OP_CLEAR
                              : VK_ATTACHMENT_LOAD_OP_LOAD,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .initialLayout = isFirst
                                     ? VK_IMAGE_LAYOUT_UNDEFINED
                                     : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .finalLayout = isLast
                                   ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
                                   : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            },
            VkAttachmentDescription{
                    .flags = 0,
                    .format= this->_deviceData.depthFormat,
                    .samples = this->_deviceData.msaaSamples,
                    .loadOp = isFirst
                              ? VK_ATTACHMENT_LOAD_OP_CLEAR
                              : VK_ATTACHMENT_LOAD_OP_LOAD,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .initialLayout = isFirst
                                     ? VK_IMAGE_LAYOUT_UNDEFINED
                                     : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                    .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            }
    };

    const VkAttachmentReference colorAttachmentReference = {
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    const VkAttachmentReference depthAttachmentReference = {
            .attachment = 1,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

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
            .pColorAttachments = &colorAttachmentReference,
            .pResolveAttachments = nullptr,
            .pDepthStencilAttachment = &depthAttachmentReference,
            .preserveAttachmentCount = 0,
            .pPreserveAttachments = nullptr
    };

    const VkRenderPassCreateInfo renderPassCreateInfo = {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = static_cast<uint32_t>(dependencies.size()),
            .pDependencies = dependencies.data()
    };

    vkEnsure(vkCreateRenderPass(this->_deviceData.device, &renderPassCreateInfo, nullptr, &this->_renderpass));
}

RenderpassBase::RenderpassBase(const RenderpassFlags &flags, const DeviceData &deviceData)
        : _flags(flags),
          _deviceData(deviceData) {
    //
}

RenderpassBase::~RenderpassBase() {
    if (this->_renderpass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(this->_deviceData.device, this->_renderpass, nullptr);
    }
}

void RenderpassBase::createFramebuffers(uint32_t width, uint32_t height, uint32_t targetImagesCount,
                                        const std::vector<std::vector<VkImageView>> &targetImageGroups) {
    this->_framebuffers.resize(targetImagesCount);

    for (uint32_t idx = 0; idx < targetImagesCount; idx++) {
        std::array<VkImageView, 2> attachments = {
                targetImageGroups[0][idx],
                targetImageGroups[1][idx]
        };

        const VkFramebufferCreateInfo framebufferCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .renderPass = this->_renderpass,
                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .width = width,
                .height = height,
                .layers = 1
        };

        vkEnsure(vkCreateFramebuffer(this->_deviceData.device, &framebufferCreateInfo, nullptr,
                                     &this->_framebuffers[idx]));
    }
}

void RenderpassBase::destroyFramebuffers() {
    for (const VkFramebuffer &framebuffer: this->_framebuffers) {
        vkDestroyFramebuffer(this->_deviceData.device, framebuffer, nullptr);
    }
}
