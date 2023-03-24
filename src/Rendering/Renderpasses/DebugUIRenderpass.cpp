#include "DebugUIRenderpass.hpp"

#include <imgui.h>
#include <imgui_impl_vulkan.h>

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
        : RenderpassBase(renderingDevice, {VkClearValue{.color = {{0, 0, 0, 0}}}}) {
    //
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
