#include "RenderpassBase.hpp"

#include "VulkanCommon.hpp"
#include "Rendering/VulkanFramebuffersBuilder.hpp"

RenderpassBase::RenderpassBase(const RenderingDevice &renderingDevice)
        : _renderingDevice(renderingDevice) {
    //
}

RenderpassBase::~RenderpassBase() {
    if (this->_renderpass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(this->_renderingDevice.device, this->_renderpass, nullptr);
    }
}

void RenderpassBase::destroyRenderpass() {
    vkDestroyRenderPass(this->_renderingDevice.device, this->_renderpass, nullptr);
}

void RenderpassBase::createFramebuffers(const Swapchain &swapchain, const RenderTargets &renderTargets) {
    this->_framebuffers = VulkanFramebuffersBuilder(this->_renderingDevice, swapchain, renderTargets,
                                                    this->_renderpass)
            .build();
}

void RenderpassBase::destroyFramebuffers() {
    for (const VkFramebuffer &framebuffer: this->_framebuffers) {
        vkDestroyFramebuffer(this->_renderingDevice.device, framebuffer, nullptr);
    }
}
