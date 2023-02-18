#include "FinalRenderpass.hpp"

#include <array>

#include "VulkanCommon.hpp"
#include "Rendering/VulkanRenderpassBuilder.hpp"
#include "Rendering/VulkanFramebuffersBuilder.hpp"

FinalRenderpass::FinalRenderpass(RenderingDevice *renderingDevice)
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

void FinalRenderpass::createFramebuffers(const Swapchain &swapchain, const RenderTargets &renderTargets) {
    this->_framebuffers = VulkanFramebuffersBuilder(this->_renderingDevice, swapchain, renderTargets,
                                                    this->_renderpass)
            .withResolveTargets()
            .build();
}
