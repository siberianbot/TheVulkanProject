#include "FramebuffersBuilder.hpp"

#include "Rendering/RenderingDevice.hpp"
#include "Rendering/Swapchain.hpp"
#include "Rendering/Objects/ImageViewObject.hpp"

FramebuffersBuilder::FramebuffersBuilder(RenderingDevice *renderingDevice,
                                         Swapchain *swapchain,
                                         VkRenderPass renderpass)
        : _renderingDevice(renderingDevice),
          _swapchain(swapchain),
          _renderpass(renderpass) {
    //
}

FramebuffersBuilder &FramebuffersBuilder::withResolveTargets() {
    _withResolveTargets = true;

    return *this;
}

std::vector<VkFramebuffer> FramebuffersBuilder::build() {
    uint32_t count = this->_swapchain->getImageCount();
    std::vector<VkFramebuffer> framebuffers(count);

    for (uint32_t idx = 0; idx < count; idx++) {
        std::vector<VkImageView> attachments = {
                this->_swapchain->getColorImageView()->getHandle(),
                this->_swapchain->getDepthImageView()->getHandle()
        };

        if (this->_withResolveTargets) {
            attachments.push_back(this->_swapchain->getSwapchainImageView(idx)->getHandle());
        }

        framebuffers[idx] = this->_renderingDevice->createFramebuffer(this->_renderpass,
                                                                      this->_swapchain->getSwapchainExtent(),
                                                                      attachments);
    }

    return framebuffers;
}
