#include "VulkanFramebuffersBuilder.hpp"
#include "VulkanCommon.hpp"

VulkanFramebuffersBuilder::VulkanFramebuffersBuilder(const RenderingDevice &renderingDevice,
                                                     const Swapchain &swapchain,
                                                     const RenderTargets &renderTargets,
                                                     VkRenderPass renderpass)
        : _renderingDevice(renderingDevice),
          _swapchain(swapchain),
          _renderTargets(renderTargets),
          _renderpass(renderpass) {
    //
}

VulkanFramebuffersBuilder &VulkanFramebuffersBuilder::withResolveTargets() {
    _withResolveTargets = true;

    return *this;
}

std::vector<VkFramebuffer> VulkanFramebuffersBuilder::build() {
    std::vector<VkFramebuffer> framebuffers(this->_swapchain.swapchainImagesCount);

    for (uint32_t idx = 0; idx < this->_swapchain.swapchainImagesCount; idx++) {
        std::vector<VkImageView> attachments = {
                this->_renderTargets.colorGroup[idx],
                this->_renderTargets.depthGroup[idx]
        };

        if (this->_withResolveTargets) {
            attachments.push_back(this->_renderTargets.resolveGroup[idx]);
        }

        const VkFramebufferCreateInfo framebufferCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .renderPass = this->_renderpass,
                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .width = this->_swapchain.width,
                .height = this->_swapchain.height,
                .layers = 1
        };

        vkEnsure(vkCreateFramebuffer(this->_renderingDevice.device, &framebufferCreateInfo, nullptr,
                                     &framebuffers[idx]));
    }

    return framebuffers;
}
