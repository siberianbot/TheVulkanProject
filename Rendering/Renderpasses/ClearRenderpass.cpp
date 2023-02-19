#include "ClearRenderpass.hpp"

#include <array>

#include "Rendering/VulkanRenderpassBuilder.hpp"

ClearRenderpass::ClearRenderpass(RenderingDevice *renderingDevice, Swapchain *swapchain)
        : RenderpassBase(renderingDevice, swapchain) {
    //
}

void ClearRenderpass::recordCommands(VkCommandBuffer commandBuffer, VkRect2D renderArea,
                                     uint32_t frameIdx, uint32_t imageIdx) {
    const std::array<VkClearValue, 2> clearValues = {
            VkClearValue{.color = {{0, 0, 0, 1}}},
            VkClearValue{.depthStencil = {1, 0}}
    };

    const VkRenderPassBeginInfo renderPassBeginInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = this->_renderpass,
            .framebuffer = this->_framebuffers[imageIdx],
            .renderArea = renderArea,
            .clearValueCount = static_cast<uint32_t>(clearValues.size()),
            .pClearValues = clearValues.data()
    };

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(commandBuffer);
}

void ClearRenderpass::initRenderpass() {
    this->_renderpass = VulkanRenderpassBuilder(this->_renderingDevice)
            .clear()
            .build();
}
