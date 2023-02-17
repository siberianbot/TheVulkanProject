#include "RenderpassBase.hpp"

#include "VulkanCommon.hpp"

RenderpassBase::RenderpassBase(const RenderingDevice &renderingDevice)
        : _renderingDevice(renderingDevice) {
    //
}

RenderpassBase::~RenderpassBase() {
    if (this->_renderpass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(this->_renderingDevice.device, this->_renderpass, nullptr);
    }
}

void RenderpassBase::destroyRenderpass() {
    vkDestroyRenderPass(this->_renderingDevice.device, this->_renderpass, nullptr);
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

        vkEnsure(vkCreateFramebuffer(this->_renderingDevice.device, &framebufferCreateInfo, nullptr,
                                     &this->_framebuffers[idx]));
    }
}

void RenderpassBase::destroyFramebuffers() {
    for (const VkFramebuffer &framebuffer: this->_framebuffers) {
        vkDestroyFramebuffer(this->_renderingDevice.device, framebuffer, nullptr);
    }
}
