#include "ClearRenderpass.hpp"

#include <array>

#include "src/Rendering/RenderpassBuilder.hpp"
#include "src/Rendering/Builders/AttachmentBuilder.hpp"
#include "src/Rendering/Builders/SubpassBuilder.hpp"

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
    this->_renderpass = RenderpassBuilder(this->_renderingDevice)
            .addAttachment([](AttachmentBuilder &builder) { builder.defaultColorAttachment(true); })
            .addAttachment([](AttachmentBuilder &builder) { builder.defaultDepthAttachment(true); })
            .addSubpass([](SubpassBuilder &builder) {
                builder
                        .withColorAttachment(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
                        .withDepthAttachment(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
            })
            .addSubpassDependency(VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0,
                                  VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
            .build();
}
