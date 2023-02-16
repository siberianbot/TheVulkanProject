#include "ClearRenderpass.hpp"

#include <array>

ClearRenderpass::ClearRenderpass(const DeviceData &deviceData)
        : RenderpassBase(RENDERPASS_FIRST, deviceData) {
    //
}

void ClearRenderpass::recordCommands(VkCommandBuffer commandBuffer, uint32_t framebufferIdx, VkRect2D renderArea) {
    const std::array<VkClearValue, 2> clearValues = {
            VkClearValue{.color = {{0, 0, 0, 1}}},
            VkClearValue{.depthStencil = {1, 0}}
    };

    const VkRenderPassBeginInfo renderPassBeginInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = this->_renderpass,
            .framebuffer = this->_framebuffers[framebufferIdx],
            .renderArea = renderArea,
            .clearValueCount = static_cast<uint32_t>(clearValues.size()),
            .pClearValues = clearValues.data()
    };

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(commandBuffer);
}
