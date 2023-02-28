#include "FinalRenderpass.hpp"

#include "src/Rendering/RenderpassBuilder.hpp"
#include "src/Rendering/FramebuffersBuilder.hpp"
#include "src/Rendering/Builders/AttachmentBuilder.hpp"
#include "src/Rendering/Builders/SubpassBuilder.hpp"

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
    this->_renderpass = RenderpassBuilder(this->_renderingDevice)
            .addAttachment([](AttachmentBuilder &builder) { builder.defaultColorAttachment(false); })
            .addAttachment([](AttachmentBuilder &builder) { builder.defaultDepthAttachment(false); })
            .addAttachment([](AttachmentBuilder &builder) { builder.defaultResolveAttachment(); })
            .addSubpass([](SubpassBuilder &builder) {
                builder
                        .withColorAttachment(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
                        .withDepthAttachment(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
                        .withResolveAttachment(2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            })
            .addSubpassDependency(VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0,
                                  VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
            .build();
}

void FinalRenderpass::createFramebuffers() {
    this->_framebuffers = FramebuffersBuilder(this->_renderingDevice, this->_swapchain, this->_renderpass)
            .withDepthTargets()
            .withResolveTargets()
            .build();
}
