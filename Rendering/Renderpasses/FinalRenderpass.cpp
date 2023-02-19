#include "FinalRenderpass.hpp"

#include "Rendering/VulkanRenderpassBuilder.hpp"
#include "Rendering/FramebuffersBuilder.hpp"

FinalRenderpass::FinalRenderpass(RenderingDevice *renderingDevice, Swapchain *swapchain)
        : RenderpassBase(renderingDevice, swapchain) {
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

void FinalRenderpass::createFramebuffers() {
    this->_framebuffers = FramebuffersBuilder(this->_renderingDevice, this->_swapchain, this->_renderpass)
            .withResolveTargets()
            .build();
}
