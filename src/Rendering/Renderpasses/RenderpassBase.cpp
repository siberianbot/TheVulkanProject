#include "RenderpassBase.hpp"

#include "src/Rendering/RenderingDevice.hpp"

RenderpassBase::RenderpassBase(const std::shared_ptr<RenderingDevice> &renderingDevice,
                               const std::vector<VkClearValue> &clearValues)
        : _clearValues(clearValues),
          _renderingDevice(renderingDevice) {
    //
}

void RenderpassBase::beginRenderpass(VkCommandBuffer commandBuffer) {
    const VkRenderPassBeginInfo renderPassBeginInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = this->_renderpass,
            .framebuffer = this->_targetFramebuffer,
            .renderArea = this->_targetRenderArea,
            .clearValueCount = static_cast<uint32_t>(this->_clearValues.size()),
            .pClearValues = this->_clearValues.data()
    };

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void RenderpassBase::endRenderpass(VkCommandBuffer commandBuffer) {
    vkCmdEndRenderPass(commandBuffer);
}

void RenderpassBase::destroyRenderpass() {
    vkDestroyRenderPass(this->_renderingDevice->getHandle(), this->_renderpass, nullptr);
}

void RenderpassBase::destroyFramebuffers() {
    for (const auto &[imageView, framebuffer]: this->_framebuffers) {
        vkDestroyFramebuffer(this->_renderingDevice->getHandle(), framebuffer, nullptr);
    }

    this->_framebuffers.clear();
}

void RenderpassBase::setTargetImageView(const std::shared_ptr<ImageViewObject> &targetImageView, VkRect2D renderArea) {
    auto it = this->_framebuffers.find(targetImageView);

    if (it != this->_framebuffers.end()) {
        this->_targetFramebuffer = it->second;
    } else {
        this->_targetFramebuffer = this->createFramebuffer(targetImageView, renderArea.extent);
        this->_framebuffers[targetImageView] = this->_targetFramebuffer;
    }

    this->_targetRenderArea = renderArea;
}
