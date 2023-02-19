#include "RenderpassBase.hpp"

#include "Rendering/FramebuffersBuilder.hpp"
#include "Rendering/RenderingDevice.hpp"
#include "Rendering/Swapchain.hpp"

RenderpassBase::RenderpassBase(RenderingDevice *renderingDevice, Swapchain *swapchain)
        : _renderingDevice(renderingDevice),
          _swapchain(swapchain) {
    //
}

void RenderpassBase::destroyRenderpass() {
    vkDestroyRenderPass(this->_renderingDevice->getHandle(), this->_renderpass, nullptr);
}

void RenderpassBase::createFramebuffers() {
    this->_framebuffers = FramebuffersBuilder(this->_renderingDevice, this->_swapchain, this->_renderpass)
            .build();
}

void RenderpassBase::destroyFramebuffers() {
    for (const VkFramebuffer &framebuffer: this->_framebuffers) {
        this->_renderingDevice->destroyFramebuffer(framebuffer);
    }
}
