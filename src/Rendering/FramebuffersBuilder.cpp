#include "FramebuffersBuilder.hpp"

#include "RenderingDevice.hpp"
#include "Swapchain.hpp"
#include "src/Rendering/Objects/ImageViewObject.hpp"

FramebuffersBuilder::FramebuffersBuilder(RenderingDevice *renderingDevice,
                                         Swapchain *swapchain,
                                         VkRenderPass renderpass)
        : _renderingDevice(renderingDevice),
          _swapchain(swapchain),
          _renderpass(renderpass) {
    //
}

FramebuffersBuilder &FramebuffersBuilder::withDepthTargets() {
    this->_withDepthTargets = true;

    return *this;
}

FramebuffersBuilder &FramebuffersBuilder::withResolveTargets() {
    this->_withResolveTargets = true;

    return *this;
}

std::vector<VkFramebuffer> FramebuffersBuilder::build() {
    uint32_t count = this->_swapchain->getImageCount();
    std::vector<VkFramebuffer> framebuffers(count);

    for (uint32_t idx = 0; idx < count; idx++) {
        std::vector<VkImageView> attachments = {
                this->_swapchain->getColorImageView()->getHandle()
        };

        if (this->_withDepthTargets) {
            attachments.push_back(this->_swapchain->getDepthImageView()->getHandle());
        }

        if (this->_withResolveTargets) {
            attachments.push_back(this->_swapchain->getSwapchainImageView(idx)->getHandle());
        }

        framebuffers[idx] = this->_renderingDevice->createFramebuffer(this->_renderpass,
                                                                      this->_swapchain->getSwapchainExtent(),
                                                                      attachments);
    }

    return framebuffers;
}
