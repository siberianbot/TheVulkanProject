#include "FinalRenderpass.hpp"

#include <array>

#include "VulkanCommon.hpp"
#include "VulkanRenderpassBuilder.hpp"

FinalRenderpass::FinalRenderpass(const RenderingDevice &renderingDevice)
        : RenderpassBase(renderingDevice) {
    //
}

void FinalRenderpass::recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                                     uint32_t frameIdx, uint32_t imageIdx) {
    const VkRenderPassBeginInfo renderPassBeginInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = this->_renderpass,
            .framebuffer = this->_framebuffers[imageIdx],
            .renderArea = renderArea,
            .clearValueCount = 0,
            .pClearValues = nullptr
    };

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(commandBuffer);
}

void FinalRenderpass::initRenderpass() {
    this->_renderpass = VulkanRenderpassBuilder(this->_renderingDevice)
            .load()
            .addResolveAttachment()
            .build();
}

void FinalRenderpass::createFramebuffers(uint32_t width, uint32_t height, uint32_t targetImagesCount,
                                         const std::vector<std::vector<VkImageView>> &targetImageGroups) {
    this->_framebuffers.resize(targetImagesCount);

    for (uint32_t idx = 0; idx < targetImagesCount; idx++) {
        std::array<VkImageView, 3> attachments = {
                targetImageGroups[0][idx],
                targetImageGroups[1][idx],
                targetImageGroups[2][idx]
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
