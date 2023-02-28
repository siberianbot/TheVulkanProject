#include "RenderpassBase.hpp"

#include "src/Rendering/Builders/FramebuffersBuilder.hpp"
#include "src/Rendering/RenderingDevice.hpp"
#include "src/Rendering/Swapchain.hpp"
#include "src/Rendering/Objects/ImageViewObject.hpp"

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
            .addAttachment(this->_swapchain->getColorImageView()->getHandle())
            .addAttachment(this->_swapchain->getDepthImageView()->getHandle())
            .build();
}

void RenderpassBase::destroyFramebuffers() {
    for (const VkFramebuffer &framebuffer: this->_framebuffers) {
        this->_renderingDevice->destroyFramebuffer(framebuffer);
    }
}
