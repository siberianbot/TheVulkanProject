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
    this->_renderingDevice->destroyRenderpass(this->_renderpass);
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
