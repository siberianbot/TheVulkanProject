#include "DebugUIRenderpass.hpp"

#include <imgui.h>
#include <imgui_impl_vulkan.h>

#include "src/Rendering/PhysicalDevice.hpp"
#include "src/Rendering/RenderingDevice.hpp"
#include "src/Rendering/Builders/AttachmentBuilder.hpp"
#include "src/Rendering/Builders/FramebufferBuilder.hpp"
#include "src/Rendering/Builders/RenderpassBuilder.hpp"
#include "src/Rendering/Builders/SubpassBuilder.hpp"
#include "src/Rendering/Objects/ImageViewObject.hpp"

VkFramebuffer DebugUIRenderpass::createFramebuffer(const std::shared_ptr<ImageViewObject> &imageView,
                                                   VkExtent2D extent) {
    return FramebufferBuilder(this->_renderingDevice.get(), this->_renderpass)
            .withExtent(extent)
            .addAttachment(imageView->getHandle())
            .build();
}

DebugUIRenderpass::DebugUIRenderpass(const std::shared_ptr<RenderingDevice> &renderingDevice)
        : _renderingDevice(renderingDevice) {
    //
}

void DebugUIRenderpass::beginRenderpass(VkCommandBuffer commandBuffer, VkRect2D renderArea, uint32_t frameIdx,
                                        uint32_t imageIdx) {
    auto it = this->_framebuffers.find(this->_targetImageView);
    VkFramebuffer targetFramebuffer;

    if (it != this->_framebuffers.end()) {
        targetFramebuffer = it->second;
    } else {
        targetFramebuffer = this->createFramebuffer(this->_targetImageView, renderArea.extent);
        this->_framebuffers[this->_targetImageView] = targetFramebuffer;
    }

    const std::array<VkClearValue, 1> clearValues = {
            VkClearValue{.color = {{0, 0, 0, 0}}}
    };

    const VkRenderPassBeginInfo renderPassBeginInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = this->_renderpass,
            .framebuffer = targetFramebuffer,
            .renderArea = renderArea,
            .clearValueCount = static_cast<uint32_t>(clearValues.size()),
            .pClearValues = clearValues.data()
    };

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void DebugUIRenderpass::endRenderpass(VkCommandBuffer commandBuffer) {
    vkCmdEndRenderPass(commandBuffer);
}

void DebugUIRenderpass::record(VkCommandBuffer commandBuffer) {
    ImGui::Render();
    ImDrawData *drawData = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);
}

void DebugUIRenderpass::initRenderpass() {
    this->_renderpass = RenderpassBuilder(this->_renderingDevice.get())
            .addAttachment([](AttachmentBuilder &builder) {
                builder
                        .clear()
                        .withFinalLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
            })
            .addSubpass([](SubpassBuilder &builder) {
                builder.withColorAttachment(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            })
            .addSubpassDependency(VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0,
                                  VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
            .build();
}

void DebugUIRenderpass::destroyRenderpass() {
    vkDestroyRenderPass(this->_renderingDevice->getHandle(), this->_renderpass, nullptr);
}

void DebugUIRenderpass::destroyFramebuffers() {
    for (const auto &[imageView, framebuffer]: this->_framebuffers) {
        vkDestroyFramebuffer(this->_renderingDevice->getHandle(), framebuffer, nullptr);
    }

    this->_framebuffers.clear();
}

void DebugUIRenderpass::setTargetImageView(const std::shared_ptr<ImageViewObject> &targetImageView) {
    this->_targetImageView = targetImageView;
}
